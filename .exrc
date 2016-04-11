set makeprg=make\ -C\ build

set colorcolumn=110
highlight ColorColumn ctermbg=darkgray

let &path.="src/include,/usr/include/AL,"


nnoremap <F4> :make CitySimulator_run<cr>
nnoremap <Leader><F4> :!build/CitySimulator_run CitySimulator<cr>
nnoremap <F5> :make CitySimulator_tests<cr>
nnoremap <Leader><F5> :!(cmake . build && cd build/CitySimulator_tests/tests && ./CitySimulator_tests)<cr>
