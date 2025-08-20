#pragma once

//includes locking, table ops, commits, etc.
struct Operation {
u32     transaction_id;
OpType  type;
string  filename;
Page*   buffer;
int     (DB::PageCache::*file_op)(string, Page&);
};
