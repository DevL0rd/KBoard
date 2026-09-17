.pragma library

const actions = [
    { button: "a", action: "Press the focused key, hold for accents" },
    { button: "b", action: "Backspace" },
    { button: "x", action: "Space" },
    { button: "y", action: "Shift" },
    { button: "dpup", action: "Move between keys", group: "dpad" },
    { button: "leftstick", action: "Move between keys" },
    { button: "rightstick", action: "Move the text cursor" },
    { button: "leftshoulder", action: "Cursor left" },
    { button: "rightshoulder", action: "Cursor right" },
    { button: "lefttrigger", action: "Previous panel" },
    { button: "righttrigger", action: "Next panel" },
    { button: "start", action: "Enter" },
    { button: "back", action: "Hide the keyboard" }
];

const tour = ["a", "b", "x", "y", "dpright", "leftshoulder", "rightshoulder", "lefttrigger", "righttrigger", "start", "back"];

function actionFor(button) {
    const key = button.startsWith("dp") ? "dpup" : button;
    const found = actions.find(entry => entry.button === key);
    return found ? found.action : "Not used by the keyboard";
}
