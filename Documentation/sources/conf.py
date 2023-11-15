# -*- coding: utf-8 -*-
#
# OpenFX documentation build configuration file, created by
# sphinx-quickstart on Thu Nov 30 08:45:24 2017.
#
# This file is execfile()d with the current directory set to its
# containing dir.
#
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))

import subprocess, os, shutil

read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'

if read_the_docs_build:
    subprocess.call('python ../genPropertiesReference.py -r -i ../../include -o Reference/ofxPropertiesReference.rst', shell=True)
    subprocess.call('cd ../../include ; doxygen ofx.doxy', shell=True)
    ps = subprocess.Popen("git rev-parse HEAD | git ls-remote --heads origin | grep $(git rev-parse HEAD) | cut -d / -f 3",shell=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    branch_name=ps.communicate()[0]
    branch_name=branch_name.rstrip().decode('utf-8')
    print(f'Current branch is: {branch_name}')
    # Modify index.rst
    with open('index.rst.tmp','w') as fo:
        with open('index.rst') as f:
            lines = f.readlines()
            for l in lines:
                if 'readthedocs.' in l and 'openfx' in l:
                    l = l.replace('master',branch_name)
                fo.write(l)

    # Copy back
    os.remove('index.rst')
    shutil.copyfile('index.rst.tmp','index.rst')
    os.remove('index.rst.tmp')

# -- General configuration ------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#
# needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [ "breathe", "sphinxjp.themes.basicstrap" ]

breathe_projects = {
    "ofx_reference":"../doxygen_build/xml/",
}

breathe_default_project = "ofx_reference"

#breathe_projects_source = {
#    "ofx_reference" :
#        ( "../../include", [ "ofxCore.h","ofxDialog.h","ofxImageEffect.h","ofxInteract.h","ofxKeySyms.h", ] )
#        }

breathe_default_members = ('members', 'protected-members', 'private-members','undoc-members')

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
# source_suffix = ['.rst', '.md']
source_suffix = '.rst'

# The master toctree document.
master_doc = 'index'

# General information about the project.
project = u'OpenFX'
copyright = u'2023, OpenFX and contributors to the OpenFX project'
author = u'OpenFX and contributors to the OpenFX Project'

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = u'1.4'
# The full version, including alpha/beta/rc tags.
release = u'1.4+'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This patterns also effect to html_static_path and html_extra_path
exclude_patterns = []

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# If true, `todo` and `todoList` produce output, else they produce nothing.
todo_include_todos = True


# -- Options for HTML output ----------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'basicstrap'

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#
html_theme_options = {
    'header_inverse': True,
    'relbar_inverse': True,
    'inner_theme': True,
    'inner_theme_name': 'bootswatch-readable',
}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

# Custom sidebar templates, must be a dictionary that maps document names
# to template names.
#
# This is required for the alabaster theme
# refs: http://alabaster.readthedocs.io/en/latest/installation.html#sidebars
#html_sidebars = {
#    '**': [
#        'about.html',
#        'navigation.html',
#        'relations.html',  # needs 'show_related': True theme option to display
#        'searchbox.html',
#        'donate.html',
#    ]
#}


# -- Options for HTMLHelp output ------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = 'OpenFXdoc'


# -- Options for LaTeX output ---------------------------------------------

latex_elements = {
    # The paper size ('letterpaper' or 'a4paper').
    #
    # 'papersize': 'letterpaper',

    # The font size ('10pt', '11pt' or '12pt').
    #
    # 'pointsize': '10pt',

    # Additional stuff for the LaTeX preamble.
    #
    # 'preamble': '',

    # Latex figure (float) alignment
    #
    # 'figure_align': 'htbp',
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (master_doc, 'OpenFX.tex', u'OpenFX Documentation',
     [author], 'manual'),
]


# -- Options for manual page output ---------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    (master_doc, 'openfx', u'OpenFX Documentation',
     [author], 1)
]


# -- Options for Texinfo output -------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
    (master_doc, 'OpenFX', u'OpenFX Documentation',
     author, 'OpenFX', 'OpenFX Project',
     'Miscellaneous'),
]




# Example configuration for intersphinx: refer to the Python standard library.
intersphinx_mapping = {'https://docs.python.org/': None}
