var knowTypes = new Array();

function createComponent(model, parent)
{
    if (knowTypes.length == 0) {
        console.debug("INITIALIZE TYPES")
        knowTypes["unity.widgets.systemsettings.tablet.textentry"] = "import components 1.0; TextEntry {}";
        knowTypes["unity.widgets.systemsettings.tablet.togglebutton"] = "import components 1.0; ToggleButton {}";
        knowTypes["unity.widgets.systemsettings.tablet.group"] = "import components 1.0; NavigationButton {}";
        knowTypes["unity.widgets.systemsettings.tablet.sectiontitle"] = "import components 1.0; SectionTitle {}";
    }

    var modelType = new String(model.type)
    var comp

    console.debug("TYPE DISCOVERY: " + model.type)

    if (knowTypes[modelType]) {
        console.debug("VALID TYPE")
        comp = Qt.createQmlObject(knowTypes[modelType], parent, '')
    } else {
        console.debug("NAMESPACE:" + modelType)

        var dotIndex = modelType.lastIndexOf(".", 0)
        var typeName = ""
        if (dotIndex > 0) {
            typeName = modelType.slice(dotIndex)
        } else {
            typeName = modelType
        }

        var component = Qt.createComponent(typeName + '.qml');
        if (component.status == Component.Ready) {
            comp = component.createObject(parent);
            comp.dbusModel = model
        } else {
            console.log("Fail to create component: " + component.errorString())
            return null
        }
    }

    if (model.type == "unity.widgets.systemsettings.tablet.group") {
        comp.stack = pages
        comp.next = Qt.createComponent("ServiceSubPage.qml")
    }

    comp.dbusModel = model
    return comp
}

