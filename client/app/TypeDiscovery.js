var knowTypes = new Array();


function createComponentFromFile(file)
{
    var component = Qt.createComponent(file);
    if (component.status == Component.Ready) {
        return component
    } else {
        console.log("Fail to load file:" + component.errorString())
    }

    return null
}

function createObjectFromFile(file, model, parent)
{
    var comp = null
    var component = createComponentFromFile(file);
    if (component) {
        comp = component.createObject(parent,  {"dbusModel" : model});
        if (!comp) {
            console.log("Fail to create component: " + component.errorString())
            return null
        }
    }

    return comp
}

function createComponent(model, parent)
{
    if (knowTypes.length == 0) {
        knowTypes["unity.widgets.systemsettings.tablet.textentry"] = "import components 1.0; TextEntry { }";
        knowTypes["unity.widgets.systemsettings.tablet.togglebutton"] = "import components 1.0; ToggleButton { }";
        knowTypes["unity.widgets.systemsettings.tablet.sectiontitle"] = "import components 1.0; SectionTitle { }";
        knowTypes["unity.widgets.systemsettings.tablet.radiobutton"] = "import components 1.0; ListItem { selectable: true }";
    }

    var modelType = new String(model.type)
    var comp

    if (knowTypes[modelType]) {
        if (model.isInline) {
            comp = createObjectFromFile("ServiceSubGroup.qml", model, parent)
            if (comp) {
                comp.headerSource = knowTypes[modelType]
                comp.menuId = model.menuId
                comp.control = model.control
                comp.index = pages.count
                comp.load()
            } else {
                console.log("Fail to load sub-group:" + comp.errorString())
            }
        } else {
            comp = Qt.createQmlObject(knowTypes[modelType], parent, '')
            comp.dbusModel = model
        }
    } else {
        var dotIndex = modelType.lastIndexOf('.')
        var typeName = ""
        if (dotIndex > 0) {
            typeName = modelType.slice(dotIndex + 1)
        } else {
            typeName = modelType
        }
        comp = createObjectFromFile("file://" + EXTRA_COMPONENTS_DIR + "/" + typeName + ".qml", model, parent);
    }

    return comp
}
