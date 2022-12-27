char help[] = 
	"nein commander presents two panels each displaying the content of a directory.\n"
	"The panel displays the name of the current directory at the top followed by the directory filter if set.\n"
	"Below is the list of directories and files of the current directory displaying in columns: "
	"the name, the size and the last modification time. "
	"By default, directories are sorted according to the file names.\n"
	"\n"
	"Navigation:\n"
	"============\n"
	"Navigation within a panel is done using standard keys.\n"
	"Key bindings:\n"
	"  Up       - move selection to previous item\n"
	"  Down     - move selection to next item\n"
	"  PageUp   - move selection up one page\n"
	"  PageDown - move selection down one page\n"
	"  Home     - move selection to the first element in the directory\n"
	"  End      - move selection to the last element in the directory\n"
	"  Tab      - switch to the other panel\n"
	"\n"
	"Mark:\n"
	"======\n"
	"The selection is the currently highlighted file or directory.\n"
	"In order to perform operations on multiple elements, it is possible to mark files."
	"Marked files are displayed in a different colors to be distinguished from unmarked ones.\n"
	"Key bindings:\n"
	"  Ins - mark the current selection\n"
	"  +   - select a group of elements using a glob pattern\n"
	"  -   - unselect a group of elements using a glob pattern\n"
	"  *   - invert the selection by marking all unselected files and unmarking all selected files\n"
	"\n"
	"Filtering:\n"
	"===========\n"
	"It is possible to filter the content of a directory to only display files whose name match a given pattern.\n"
	"When a panel is filtered, the filter will appear between square brackets beside the directory name.\n"
	"Key bindings:\n"
	"  f - filter the directory content using a glob pattern\n"
	"\n"
	"Operations:\n"
	"============\n"
	"File operations can apply either to the currently selected element or to marked elements.\n"
	"Operations that expect a destination directory (copy, move), will use the other panel as the destination directory."
	"If the other panel has the same directory then the move operation will instead rename the current selection"
	" and copy will do nothing.\n"
	"Key bindings:\n"
	"  Enter - view file content or visit directory.\n"
	"  c     - change current directory (accepts relative or absolute paths).\n"
	"  C     - change current directory of other panel.\n"
	"  =     - change other panel directory to current one.\n"
	"  F1    - this help screen.\n"
	"  F3    - view file content or visit directory.\n"
	"  F4    - send the selected element to the plumber.\n"
	"  F5    - copy either marked elements or selection.\n"
	"  F6    - rename or move either marked elements or selection.\n"
	"  F7    - create a new directory.\n"
	"  F8    - delete marked elements or selection.\n"
	"  F10   - exit nein commander.\n"
	"\n";
