# KLIPY integration notes

Verified on 2026-09-18 against the live documentation (docs.klipy.com serves Markdown copies of every page at `<page>.md`; plain curl without a browser user agent gets 403).

## Key and limits

- Keys are created in the Partner Panel: https://partner.klipy.com/api-keys ("Add Platform").
- Testing keys are limited to 100 requests per hour; production access is requested in the Partner Panel and is unlimited.
  Sources: https://docs.klipy.com/getting-started, https://github.com/KLIPY-com/Migrate-From-Tenor-To-Klipy, https://klipy.com/migrate
- KBoard compiles the key in from the CMake cache variable `KBOARD_KLIPY_API_KEY` (default in `src/gif/CMakeLists.txt`). Empty key means `GifStore.configured == false` and the panel shows "GIFs need a KLIPY API key at build time".
- An invalid key answers HTTP 404 with `{"result":false,"errors":{"message":["The provided API key is invalid."]}}` (checked live with a bogus key).

## Endpoints used (native API, base `https://api.klipy.com/`)

| Purpose | Method and path | Parameters |
| --- | --- | --- |
| Trending GIFs / stickers | `GET api/v1/{app_key}/gifs/trending`, `.../stickers/trending` | `page`, `per_page`, `customer_id`, `locale`, `content_filter`, `format_filter` |
| Search | `GET api/v1/{app_key}/gifs/search`, `.../stickers/search` | `q`, `page`, `per_page`, `customer_id`, `locale`, `content_filter`, `format_filter` |
| Categories | `GET api/v1/{app_key}/gifs/categories`, `.../stickers/categories` | `locale` in `xx_YY` form |
| Autocomplete | `GET api/v1/{app_key}/autocomplete/{q}` | `limit` (default 10) |
| Search suggestions | `GET api/v1/{app_key}/search-suggestions/{q}` | `limit` (default 10) |
| Share trigger | `POST api/v1/{app_key}/gifs/share/{slug}`, `.../stickers/share/{slug}` | JSON body `customer_id`, `q` (the search that led to the share, empty for trending) |
| Items by slug | `GET api/v1/{app_key}/gifs/items` | `slugs` comma separated |

Sources:
https://docs.klipy.com/gifs-api/gifs-trending-api,
https://docs.klipy.com/gifs-api/gifs-search-api,
https://docs.klipy.com/gifs-api/gifs-categories-api,
https://docs.klipy.com/gifs-api/gifs-share-trigger-api,
https://docs.klipy.com/gifs-api/gifs-items-api,
https://docs.klipy.com/stickers-api/stickers-search-api,
https://docs.klipy.com/stickers-api/stickers-categories-api,
https://docs.klipy.com/stickers-api/stickers-share-trigger-api,
https://docs.klipy.com/search-suggestions-and-autocomplete/autocomplete,
https://docs.klipy.com/search-suggestions-and-autocomplete/search-suggestions

KLIPY also offers a Tenor-compatible `https://api.klipy.com/v2/search?key=...` drop-in (https://klipy.com/migrate). KBoard uses the native API above instead.

## Parameters

- Pagination is page based, not cursor based: request `page` and `per_page`, response has `data.current_page`, `data.per_page`, `data.has_next`.
- `content_filter`: `off`, `low`, `medium`, `high` (maps 1:1 to `Settings.gifContentFilter`). Filters follow MPA-style ratings; extra blocking is configured in the Partner Panel. https://docs.klipy.com/content-filtering
- `customer_id`: optional but documented as "a unique user identifier ... consistent for the same user"; used for personalisation, per-user recents and share analytics. KBoard generates a random UUID once and stores it in `~/.local/share/kboard/gif/customer_id`.
- `locale`: trending and search document an ISO 3166-1 alpha-2 country code (`us`, `ge`); categories document `xx_YY` (`en_US`). Search results are also localised from the query language. https://docs.klipy.com/migrate-from-tenor/localization
- `format_filter`: comma separated subset of `gif, webp, jpg, mp4, webm`. KBoard asks for `gif,webp`.

## Response shape

```
{ "result": true,
  "data": { "data": [ { "id": 8041071659142944, "slug": "hello-hi-662", "title": "Hello",
                        "file": { "hd": {"gif": {url,width,height,size}, "webp": ..., "jpg": ..., "mp4": ..., "webm": ...},
                                  "md": {...}, "sm": {...}, "xs": {...} },
                        "tags": [], "type": "gif", "blur_preview": "data:image/jpeg;base64,..." } ],
            "current_page": 1, "per_page": 24, "has_next": true } }
```

Stickers use the same shape with `type: "sticker"` and `gif`, `webp`, `webm`, `png` per size. Categories: `data.locale`, `data.categories[] {category, query, preview_url}`. Autocomplete and suggestions: `data` is an array of strings.

Size tiers (https://docs.klipy.com/gifs-api/gifs-format-sizes): `xs` about 90 px (webp median 35 KB), `sm` about 220 px (webp median 117 KB, gif 206 KB), `md` (gif median 1.4 MB), `hd` (gif median 2.6 MB).

KBoard picks: grid preview `sm.webp` (animated WebP, smallest animated format Qt decodes), paste file `md.gif`, link mode `hd.gif`, placeholder `blur_preview`.

## Attribution (https://docs.klipy.com/attribution)

- REQUIRED: "Search KLIPY" as the default placeholder of the search field. Done (`GifStore.searchPlaceholder`).
- OPTIONAL: "Powered by KLIPY" mark wherever KLIPY content is shown. Done as a badge in the panel (`GifStore.attribution`).
- OPTIONAL: KLIPY watermark on shared content cards (not applicable to a keyboard).
- Official logo assets: https://drive.google.com/drive/u/3/folders/1ix5_5221kgbJHPqhCxwPsqHlHexhQP2w

## Integration requirements that affect KBoard (https://docs.klipy.com/integration-requirements)

- Use media URLs exactly as returned; load media directly from KLIPY from the user's device; no proxies. KBoard does this.
- Keep Search and Trending results in the order returned and do not filter them client side. KBoard only drops entries that carry no usable media, it never reorders.
- KLIPY content must stay in its own grid, not mixed with other providers. KBoard has no other provider.
- "Do not store, mirror, re-host, rewrite, or retain copies of KLIPY media unless KLIPY has approved a different delivery method in writing." Pasting a GIF into another app needs a local file for the clipboard, so KBoard writes the chosen GIF to `~/.cache/kboard/gif/` (LRU trimmed to `Settings.gifCacheSizeMb`). This needs confirmation from developers@klipy.com, together with the embedded-key question from the build plan.
- Share trigger: recommended for personalisation and analytics; KBoard calls it after every successful insert.
