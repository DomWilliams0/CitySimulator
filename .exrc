set makeprg=make\ CitySimulator_run\ -C\ build\ -j4

set colorcolumn=110
highlight ColorColumn ctermbg=darkgray

let &path.="src/include,/usr/include/AL,"

nnoremap <Leader><F4> :!build/CitySimulator_run CitySimulator<cr>
