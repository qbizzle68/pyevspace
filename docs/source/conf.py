# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'PyEVSpace'
copyright = '2023-2026, Quinton Barnes'
author = 'Quinton Barnes'

import tomllib
import pathlib

path = pathlib.Path(__file__)
with open(path.parents[2] / 'pyproject.toml', 'rb') as f:
    toml = tomllib.load(f)

__version__ = toml['project']['version']
version = __version__
release = __version__

html_title = f"PyEVSpace {version}"
html_show_sourcelink = False

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "sphinx.ext.intersphinx",
]

templates_path = ['_templates']
exclude_patterns = []
intersphinx_mapping = {
    "python": ("https://docs.python.org/3", None),
}



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'pydata_sphinx_theme'
html_static_path = ["_static"]
html_css_files = ["css/custom-styles.css"]
html_theme_options = {
    "show_nav_level": 2,
    "navigation_depth": 3,
    "icon_links": [
        {
            "name": "GitHub",
            "url": "https://github.com/qbizzle68/pyevspace",
            "icon": "fa-brands fa-github",
        }
    ]
}
