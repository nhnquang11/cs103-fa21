RUN_TESTS_MENU_OPTION()

WINDOW_TITLE("CS103: Problem Set Two")

MENU_ORDER("InterpersonalDynamicsGUI.cpp",
           "PropositionalCompletenessGUI.cpp",
           "ExecutableLogicGUI.cpp",
           "ThisButNotThatGUI.cpp",
           "TranslatingIntoLogicGUI.cpp")

TEST_ORDER("InterpersonalDynamicsTests.cpp",
           "PropositionalCompletenessTests.cpp",
           "ExecutableLogicTests.cpp",
           "FirstOrderNegationsTests.cpp",
           "ThisButNotThatTests.cpp",
           "TranslatingIntoLogicTests.cpp")
