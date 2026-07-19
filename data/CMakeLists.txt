
########### install files ###############
if (UNIX)
INSTALL(FILES default_variable_stars.txt default_deepsky_objects.fab default_landscapes.ini default_stars.ini mcities.fab default_anchor.ini zone.tab default_config.ini default_ssystem.ini default_joypad.ini  name.fab  DESTINATION share/${PACKAGE}/data)
# Desktop entry (beta packaging 2026-07-19). No Icon= yet: the repo ships no
# icon asset - add one (share/icons/hicolor) and the Icon= line together.
INSTALL(FILES spacecrafter.desktop DESTINATION share/applications)
else()
INSTALL(FILES default_variable_stars.txt default_deepsky_objects.fab default_landscapes.ini default_stars.ini mcities.fab default_anchor.ini zone.tab default_config.ini default_ssystem.ini default_joypad.ini  name.fab  DESTINATION data)
endif()
