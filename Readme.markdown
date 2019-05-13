QLMarkdown
==========

by Phil Toland

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

If the newly installed plugin is not picked up instantly, you can run `qlmanage -r` in Terminal to refresh.

To uninstall, drag QLMarkdown into the trash.

Another method, provided you have [Homebrew Cask](https://caskroom.github.io) installed, you can run the following commands to set things up:

`$ brew update`

`$ brew cask install qlmarkdown`

To uninstall:

`$ brew cask uninstall qlmarkdown`


 **Note:** *QuickLook doesn't allow selecting text by default. If you want to select the text in the markdown preview, you will 
need to enable text selection in QuickLook by running the following command in Terminal:*

`defaults write com.apple.finder QLEnableTextSelection -bool TRUE; killall Finder`

*This command is only valid on OS X 10.10 and below as Apple has since [removed][] the ability to select text in QuickLook.*

[removed]: http://tidbits.com/article/16254

Downloads
---------

Source code is available at <http://github.com/toland/qlmarkdown>.

You can download the [latest](https://github.com/toland/qlmarkdown/releases/latest) release from 
<https://github.com/toland/qlmarkdown/releases>

License
-------

The QLMarkdown code is distributed under the same terms as [Discount][Discount]. See the file `discount/COPYRIGHT` for more information.


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
