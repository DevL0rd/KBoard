#!/usr/bin/env python3
import os

import dbus
import dbus.service
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GLib

INTERFACE = "org.devl0rd.KBoard"


class FakeKeyboard(dbus.service.Object):
    def __init__(self, bus):
        super().__init__(bus, "/KBoard")
        self.visible = False
        self.panel = "keys"

    def called(self, *words):
        with open(os.environ["KBOARD_FAKE_CALLS"], "a") as log:
            log.write(" ".join(words) + "\n")

    def set_visible(self, visible):
        if visible != self.visible:
            self.visible = visible
            self.VisibleChanged(visible)

    @dbus.service.signal(INTERFACE, signature="b")
    def VisibleChanged(self, visible):
        pass

    @dbus.service.signal(INTERFACE, signature="s")
    def PanelChanged(self, panel):
        pass

    @dbus.service.method(INTERFACE, out_signature="b")
    def IsVisible(self):
        return self.visible

    @dbus.service.method(INTERFACE, out_signature="s")
    def CurrentPanel(self):
        return self.panel

    @dbus.service.method(INTERFACE)
    def Show(self):
        self.called("Show")
        self.set_visible(True)

    @dbus.service.method(INTERFACE)
    def Hide(self):
        self.called("Hide")
        self.set_visible(False)

    @dbus.service.method(INTERFACE)
    def Toggle(self):
        self.called("Toggle")
        self.set_visible(not self.visible)

    @dbus.service.method(INTERFACE, in_signature="s")
    def OpenPanel(self, name):
        self.called("OpenPanel", name)
        if name != self.panel:
            self.panel = name
            self.PanelChanged(name)
        self.set_visible(True)

    @dbus.service.method(INTERFACE, in_signature="s")
    def OpenSettings(self, page):
        self.called("OpenSettings", repr(str(page)))


def main():
    DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName(INTERFACE, bus)
    keyboard = FakeKeyboard(bus)
    keyboard.called("ready", name.get_name())
    GLib.MainLoop().run()


if __name__ == "__main__":
    main()
