# Table Documentation

## How rows are stored?
Each row is identified with a row id. This row id consists of (Page Id, Slot Id). This tells us which page the row is physically stored in and the offset to search for the certain set of bytes in the page. Now that we know how to find rows given a row id how do we store a row?
<br>
The Table class contains a list of pages with free space called freePages. This essentially tells us which pages can have more rows inserted into it, and if there aren't any pages that are free then the Table requests a new page for the row. Frequently, when rows get deleted these add new slots for new rows to be in, so the freePage list needs to be updated with the new Page. 