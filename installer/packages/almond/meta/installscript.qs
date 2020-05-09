function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    component.createOperations();
    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/Almond.exe", "@StartMenuDir@/Almond.lnk",
            "workingDirectory=@TargetDir@", "iconPath=@TargetDir@/almond.ico", "description=Open Almond");
    }
}
