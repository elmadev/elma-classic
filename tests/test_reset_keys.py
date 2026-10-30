import re
import pathlib
import sys

settings_h = sys.argv[1]
settings_cpp = sys.argv[2]

# Find keys in settings
settings_h_txt = pathlib.Path(settings_h).read_text(encoding="utf-8")
settings_keys = re.findall(r"DECLARE_SETTING\((.*_key.*)\)", settings_h_txt)
if not settings_keys:
    print("Could not find any settings keys!")
    sys.exit(1)

# Find reset keys in eol_settings::reset_keys()
settings_cpp_txt = pathlib.Path(settings_cpp).read_text(encoding="utf-8")
settings_reset = re.findall(r"RESET_KEY\((.*_key.*)\)", settings_cpp_txt)
if not settings_reset:
    print("Could not find any keys in eol_settings::reset_keys()!")
    sys.exit(1)

# Check duplicate keys
if len(settings_reset) != len(set(settings_reset)):
    print("Duplicate keys in eol_settings::reset_keys()!")
    sys.exit(1)

# Check keys
missing = set(settings_keys) - set(settings_reset)
if missing:
    print("Could not find the following keys in eol_settings::reset_keys():")
    for key in sorted(missing):
        print(f"  {key}")
    sys.exit(1)
