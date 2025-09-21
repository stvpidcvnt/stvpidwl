set number
set nocompatible
set wrap
set termguicolors

let NERDTreeShowHidden=1

colorscheme catppuccin_mocha
let g:airline_theme = 'catppuccin_mocha'

if filereadable(expand("~/.vimrc.plug"))
  source ~/.vimrc.plug
endif
