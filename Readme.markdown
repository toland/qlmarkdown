QLMarkdown
==========

by Phil Toland
<http://fiatdev.com>

fixes and improvements by Michael Dominic K.
<http://www.mdk.org.pl>

Introduction
------------

QLMarkdown is a simple QuickLook generator for Markdown files. It renders a
preview of the selected Markdown file using [Discount][Discount] -- a C implementation of 
John Gruber's Markdown.pl script.

To update to the latest version of discount run `./discount-config/update-discount.sh`

For more information on Markdown see 
<http://daringfireball.net/projects/markdown/>.


Installation
------------

Simply copy QLMarkdown.qlgenerator to `~/Library/QuickLook` or `/Library/QuickLook`.

If the newly installed plugin is not picked up instantly, you run `qlmanage -r` in a terminal to refresh.

To uninstall, drag QLMarkdown into the trash.

 **Note:** *Quick Look doesn't allow selecting text. If you want to select the text in the markdown preview, you will 
need to enable text selection in Quick Look by running the following in Terminal:*

````bash
defaults write com.apple.finder QLEnableTextSelection -bool TRUE; killall Finder
````

Downloads
---------

Source code is available at <http://github.com/toland/qlmarkdown>.

You can download the [latest](https://github.com/toland/qlmarkdown/releases/tag/v1.3.1) release from 
<https://github.com/toland/qlmarkdown/releases>

License
-------

The QLMarkdown code is distributed under the same terms as [Discount][Discount]. See
the file `discount/COPYRIGHT` for more information.


Version History
---------------
Version 1.3 - Jan 26, 2012

* Major update of discount markdown engine

Version 1.2 - Oct 4, 2009

* Work around a conflict with MacVim (thanks to godDLL)
* Support for .mdml extension (alanhogan)
* CSS that mimics Apple's ADC styling (jiho)

Version 1.1 - Feb 11, 2009

* Adding a little bit of CSS styling. (mdk)
* Replace the Perl markdown renderer with a native C one (discount). (mdk)
* Conform to public plain-text. Will make spotlight index the file
  contents. (mdk) 
* Added support for .md file extension (sant0sk1)

Version 1.0 - July 15, 2008

* Initial release.

[Discount]: http://www.pell.portland.or.us/~orc/Code/markdown/
