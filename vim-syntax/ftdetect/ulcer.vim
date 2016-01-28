au BufNewFile,BufRead *.ul setf ulcer
au BufNewFile,BufRead *.ulcer setf ulcer
au BufNewFile,BufRead Jakefile setf ulcer

fun! s:Selectulcer()
  if getline(1) =~# '^#!.*/bin/env\s\+node\>'
    set ft=ulcer
  endif
endfun
au BufNewFile,BufRead * call s:Selectulcer()