set colorcolumn=110
highlight ColorColumn ctermbg=darkgray

let &path.="src/include,/usr/include/AL,"

nnoremap <F4> :!./build.sh make run<cr>
nnoremap <F5> :!./build.sh make tests<cr>
nnoremap <Leader><F4> :!./build.sh run run<cr>
nnoremap <Leader><F5> :!./build.sh run tests<cr>
nnoremap <Leader><Leader><F4> :!./build.sh all run<cr>
nnoremap <Leader><Leader><F5> :!./build.sh all tests<cr>
