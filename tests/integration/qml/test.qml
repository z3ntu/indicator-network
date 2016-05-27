import Ubuntu.Connectivity 1.1

import QtQuick 2.4
import QtQuick.Layouts 1.2

import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3 as ListItem

MainView {
    width: units.gu(48)
    height: units.gu(60)

    Page {
        title: "Cellular Information"

        SortFilterModel {
            id: sortedModems
            model: Connectivity.modems
            sort.property: "Index"
            sort.order: Qt.AscendingOrder
        }

        ColumnLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: units.gu(2)
            anchors.margins: units.gu(2)

            RowLayout {
                Layout.topMargin: units.gu(2)
                Label {
                    text: i18n.tr("Cellular data")
                    Layout.fillWidth: true
                }
                Switch {
                    checked: Connectivity.mobileDataEnabled
                    function trigger() {
                        Connectivity.mobileDataEnabled = !checked
                    }
                }
            }
            ColumnLayout {
                Layout.leftMargin: units.gu(2)
                spacing: units.gu(2)
                OptionSelector {
                    id: modemSelector
                    expanded: currentSim === null
                    model: sortedModems
                    selectedIndex: -1

                    delegate: OptionSelectorDelegate {
                        text: {
                            if (model.Sim) {
                                circled(model.Index) + " " + model.Sim.PrimaryPhoneNumber
                            }
                            else {
                                return circled(model.Index) + " " + i18n.tr("No SIM detected")
                            }
                        }
                        subText: {
                            if (model.Sim) {
                                return ""
                            }
                            else {
                                return i18n.tr("Insert a SIM, then restart the phone.")
                            }
                        }
                        //enabled: model.Sim !== null // https://bugs.launchpad.net/ubuntu/+source/ubuntu-ui-toolkit/+bug/1577359

                        function circled(index) {
                            if (index === 1) {
                                return "①"
                            } else if (index === 2) {
                                return "②"
                            }

                            return " "
                        }
                    }

                    property var currentSim : null
                    onSelectedIndexChanged: {
                        if (selectedIndex == -1) {
                            currentSim = null
                        } else {
                            currentSim = model.get(selectedIndex).Sim
                        }
                    }
                    onCurrentSimChanged: {
                        if (currentSim != null) {
                            dataRoamingSwitch.checked = Qt.binding(function() {
                                return currentSim.DataRoamingEnabled
                            })
                        }
                    }

                    function setSelectedIndex() {
                        if (Connectivity.simForMobileData === null) {
                            modemSelector.selectedIndex = -1
                            return
                        }

                        for (var i = 0; i < sortedModems.count; i++) {
                            if (sortedModems.get(i).Sim === Connectivity.simForMobileData) {
                                modemSelector.selectedIndex = i
                                return
                            }
                        }
                        modemSelector.selectedIndex = -1
                    }
                    Connections {
                        target: Connectivity
                        onSimForMobileDataUpdated: {
                            modemSelector.setSelectedIndex()
                        }
                    }
                    Component.onCompleted: {
                        setSelectedIndex()
                    }

                    onTriggered:  {
                        // @bug: https://bugs.launchpad.net/ubuntu/+source/ubuntu-ui-toolkit/+bug/1577351
                        // Connectivity.simForMobileData = currentSim
                    }
                    onDelegateClicked: {
                        var sim = sortedModems.get(index).Sim
                        if (sim === null) {
                            return
                        }
                        Connectivity.simForMobileData = sim
                    }
                }
                RowLayout {
                    Label {
                        text: i18n.tr("Data roaming")
                        Layout.fillWidth: true
                    }
                    Switch {
                        id: dataRoamingSwitch
                        enabled: modemSelector.currentSim !== null
                        checked: modemSelector.currentSim ? modemSelector.currentSim.DataRoamingEnabled : false
                        function trigger() {
                            modemSelector.currentSim.DataRoamingEnabled = !checked
                        }
                    }
                }
            }

            OptionSelector {
                expanded: true
                text: "Sims"
                model: Connectivity.sims
                selectedIndex: -1
                delegate: OptionSelectorDelegate {
                    text: model.Imsi + ": " + model.PrimaryPhoneNumber

                }
                onSelectedIndexChanged: {

                }
            }
        }
    }
}
