
# list these here to have them visible in Qt Creator
set(AUTOPILOT_FILES
    indicator_network/tests/__init__.py
    indicator_network/tests/test_lock_screen.py
    indicator_network/tests/test_unlock_sim.py
    indicator_network/tests/test_notifications.py
    indicator_network/tests/PhonesimManager.py
)
add_custom_target(QtCreatorAutopilotHack SOURCES ${AUTOPILOT_FILES})
