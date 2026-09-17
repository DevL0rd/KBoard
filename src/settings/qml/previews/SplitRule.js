.pragma library

const Auto = 0;
const Always = 1;
const Never = 2;

function shouldSplit(mode, screenWidth, screenHeight, heightShare, rows) {
    if (mode === Always) {
        return true;
    }
    if (mode === Never) {
        return false;
    }
    const aspect = screenWidth / screenHeight;
    const keyWidth = screenWidth / 10;
    const keyHeight = screenHeight * heightShare / rows;
    return aspect >= 1.6 && keyWidth > keyHeight * 1.5;
}
