with section("parse"):
    additional_commands = {
        "pybind11_add_module": {
            "pargs": "1+",
            "flags": [],
            "kwargs": {
                "MODULE": 1,
                "SHARED": 1,
                "EXCLUDE_FROM_ALL": 1,
                "NO_EXTRAS": 1,
                "SYSTEM": 1,
                "THIN_LTO": 1,
                "OPT_SIZE": 1,
            },
        },
        "setup_target_for_coverage_lcov": {
            "pargs": 0,
            "flags": ["NO_DEMANGLE"],
            "kwargs": {
                "NAME": 1,
                "BASE_DIRECTORY": 1,
                "EXCLUDE": "*",
                "EXECUTABLE": "*",
                "EXECUTABLE_ARGS": "*",
                "DEPENDENCIES": "*",
                "LCOV_ARGS": "*",
                "GENHTML_ARGS": "*",
            },
        },
        "setup_target_for_coverage_gcovr_xml": {
            "pargs": 0,
            "flags": [],
            "kwargs": {
                "NAME": 1,
                "BASE_DIRECTORY": 1,
                "EXCLUDE": "*",
                "EXECUTABLE": "*",
                "EXECUTABLE_ARGS": "*",
                "DEPENDENCIES": "*",
            },
        },
        "setup_target_for_coverage_gcovr_html": {
            "pargs": 0,
            "flags": [],
            "kwargs": {
                "NAME": 1,
                "BASE_DIRECTORY": 1,
                "EXCLUDE": "*",
                "EXECUTABLE": "*",
                "EXECUTABLE_ARGS": "*",
                "DEPENDENCIES": "*",
            },
        },
    }

with section("lint"):
    disabled_codes = [
        # A custom command with one output doesn't really need a comment because
        # the default "generating XXX" is a good message already.
        "C0113",
    ]

with section("format"):
    dangle_parens = False
    line_ending = "unix"
    line_width = 120
    max_pargs_hwrap = 3
    separate_ctrl_name_with_space = False
    separate_fn_name_with_space = False
    tab_size = 2
