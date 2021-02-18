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
        "add_argus_component": {
            "pargs": "1+",
        },
    }

with section("lint"):
    disabled_codes = [
        "C0113",
    ]

with section("format"):
    dangle_parens = True
    line_ending = "unix"
    line_width = 80
    tab_size = 2
    keyword_case = "upper"

with section("markup"):
    bullet_char = "-"
