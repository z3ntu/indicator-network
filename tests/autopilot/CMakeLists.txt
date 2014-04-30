
# list these here to have them visible in Qt Creator
set(AUTOPILOT_FILES
    indicator-network/tests/__init__.py
    indicator-network/tests/test_lock_screen.py
    indicator-network/tests/test_unlock_sim.py
    indicator-network/tests/test_notifications.py
    indicator-network/tests/PhonesimManager.py
)
add_custom_target(QtCreatorAutopilotHack SOURCES ${AUTOPILOT_FILES})
