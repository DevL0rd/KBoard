.pragma library

function typeOf(pad) {
    return pad && pad.connected ? pad.controllerType : "xbox";
}

function label(pad, button) {
    return pad ? pad.glyphFor(typeOf(pad), button) : button;
}
