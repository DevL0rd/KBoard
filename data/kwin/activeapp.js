function reportActiveApp(window) {
    callDBus("org.devl0rd.KBoard", "/KBoard", "org.devl0rd.KBoard", "SetActiveApp", window && !window.deleted ? window.resourceClass : "");
}

workspace.windowActivated.connect(reportActiveApp);
reportActiveApp(workspace.activeWindow);
