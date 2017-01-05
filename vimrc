" Basics {
set nocompatible " get out of horrible vi-compatible mode *Vundle required* filetype off " *Vundle required*
" Vundle
set rtp+=~/.vim/bundle/vundle/
call vundle#begin()
Plugin 'gmarik/Vundle.vim'
Plugin 'scrooloose/nerdtree'
Plugin 'scrooloose/syntastic'
Plugin 'scrooloose/nerdcommenter'
Plugin 'kien/ctrlp.vim'
Plugin 'rking/ag.vim'
Plugin 'majutsushi/tagbar'
Plugin 'junegunn/vim-easy-align'
Plugin 'mattn/emmet-vim'
Plugin 'tpope/vim-surround'
Plugin 'ervandew/supertab'
Plugin 'honza/vim-snippets'
Plugin 'Raimondi/delimitMate'
Plugin 'pangloss/vim-javascript'
Plugin 'kchmck/vim-coffee-script'
Plugin 'groenewege/vim-less'
Plugin 'matchit.zip'
call vundle#end()
filetype plugin indent on " load filetype plugins and indent settings; *Vundle required*
set cindent shiftwidth=4
" end Vundle

set background=dark " we are using a dark background
set t_Co=16 " color numbers
set encoding=utf-8
set enc=utf-8
set fenc=utf-8
set langmenu=zh_CN,UTF-8
syntax on " syntax highlighting on
syntax enable

colorscheme evening
""colorscheme solarized
""set guifont=Monaco:h14

" When editing a file, always jump to the last cursor position


let NERDTreeIgnore=['\.pyc', '\~$', '\.git', '\.hg', '\.svn', '\.dsp', '\.opt', '\.plg', '\.pdf']

" Mark trailing spaces
match ErrorMsg '\s\+$'

" Remove trailing spaces
function! TrimTrailingSpaces()
    %s/\s\+$//e
endfunction

"autocmd FileWritePre     * call TrimTrailingSpaces()
"autocmd FileAppendPre    * call TrimTrailingSpaces()
"autocmd FilterWritePre   * call TrimTrailingSpaces()
"autocmd BufWritePre      * call TrimTrailingSpaces()

" General {
set history=1000 " How many lines of history to remember
set clipboard+=unnamed " turns out I do like is sharing windows clipboard
set fileformats=unix,dos,mac " support all three, in this order
set viminfo+=! " make sure it can save viminfo
set iskeyword+=_,$,@,%,# " none of these should be word dividers, so make them not be
set nostartofline " leave my cursor where it was
let mapleader=","
set wrap
" }

" Files/Backups {
set sessionoptions+=globals " What should be saved during sessions being saved
set sessionoptions+=localoptions " What should be saved during sessions being saved
set sessionoptions+=resize " What should be saved during sessions being saved
set sessionoptions+=winpos " What should be saved during sessions being saved
" }
"
" The Silver Searcher
if executable('ag')
  " Use ag over grep
  set grepprg=ag\ --nogroup\ --nocolor

  " Use ag in CtrlP for listing files. Lightning fast and respects .gitignore
  let g:ctrlp_user_command = 'ag %s -l --nocolor -g ""'

  " ag is fast enough that CtrlP doesn't need to cache
  let g:ctrlp_use_caching = 0
endif

let b:syntastic_mode = 'passive'
let g:syntastic_javascript_checkers = ['standard']
let g:syntastic_html_checkers=['']
let g:syntastic_check_on_open = 0
let g:syntastic_check_on_wq = 0
noremap <C-c> :SyntasticCheck<CR>

" Vim UI {
set popt+=syntax:y " Syntax when printing
set showcmd " show the command being typed
set linespace=0 " space it out a little more (easier to read)
set wildmenu " turn on wild menu
set wildmode=list:longest " turn on wild menu in special format (long format)
set wildignore=*.pyc,*.pyo,*.dll,*.o,*.obj,*.exe,*.swo,*.swp,*.jpg,*.gif,*.png " ignore some formats,*.bak,
set ruler " Always show current positions along the bottom
set cmdheight=1 " the command bar is 1 high
set number " turn on line numbers
set numberwidth=4 " If we have over 9999 lines, ohh, boo-hoo
set lazyredraw " do not redraw while running macros (much faster) (LazyRedraw)
set hidden " you can change buffer without saving
set backspace=2 " make backspace work normal
set whichwrap+=<,>,[,],h,l  " backspace and cursor keys wrap to
"set mouse=a " use mouse everywhere
"set shortmess=atI " shortens messages to avoid 'press a key' prompt
set report=0 " tell us when anything is changed via :...
set noerrorbells " don't make noise

" highlight the cursor current line in current window
autocmd VimEnter,WinEnter,BufWinEnter * setlocal cursorline
autocmd WinLeave * setlocal nocursorline
highlight CursorLine ctermbg=black cterm=bold
highlight LineNr ctermbg=darkgrey ctermfg=grey " line number bg and fg schema
"
set list listchars=tab:\ \ ,trail:· " mark trailing white space
" }

" Visual Cues {
set showmatch " show matching brackets
set matchtime=5 " how many tenths of a second to blink matching brackets for
set hlsearch
set incsearch " BUT do highlight as you type you search phrase
set scrolloff=5 " Keep 5 lines (top/bottom) for scope
set sidescrolloff=5 " Keep 5 lines at the size
"set novisualbell " don't blink
set vb " blink instead beep
set statusline=%f%m%r%h%w\ [POS=%04l,%04v][%p%%]\ [LEN=%L]
"set statusline=%F%m%r%h%w\ [FORMAT=%{&ff}]\ [TYPE=%Y]\ [ENCODE=%{&fenc}]\ [POS=%04l,%04v][%p%%]\ [LEN=%L]
set laststatus=2 " always show the status line
" }

" Indent Related {
set autoindent " autoindent (should be overwrote by cindent or filetype indent)
set smartindent
set cindent
set noexpandtab
set linespace=0
set tabstop=4 " real tabs should be 4, but they will show with set list on
set softtabstop=4
set shiftwidth=4
set smarttab " be smart when using tabs
set backspace=2

" }

" Text Formatting/Layout {
"set formatoptions=tcrq " See Help (complex)
set shiftround " when at 3 spaces, and I hit > ... go to 4, not 5
""set nowrap " do not wrap line
set preserveindent " but above all -- follow the conventions laid before us
set ignorecase " case insensitive by default
set smartcase " if there are caps, go case-sensitive
set completeopt=menu,longest,preview " improve the way autocomplete works
set nocursorcolumn " don't show the current column
" }

" Folding {
set foldenable        " Turn on folding
"set foldmarker={,}        " Fold C style code (only use this as default if you use a high foldlevel)
"set foldcolumn=4        " Give 1 column for fold markers
""set foldopen-=search    " don't open folds when you search into them
""set foldopen-=undo        " don't open folds when you undo stuff
set foldmethod=indent   " Fold on the marker
"set foldnestmax=2
set foldlevel=1000 " Don't autofold anything (but I can still fold manually)
""" }
""

" SuperTab
let g:SuperTabDefaultCompletionType = "<C-x><C-o>"
let g:SuperTabDefaultCompletionType = "context"

" Mappings {
noremap <Leader>t :call TrimTrailingSpaces()<CR>
" Count number of matches
noremap <Leader>c :%s///gn<CR>
noremap <Leader>a ^
noremap <Leader>e $
noremap <Leader>s :w<CR>
noremap <Leader>z :q<CR>
noremap <Leader>r :NERDTreeFind<CR>

inoremap ' ''<ESC>i
inoremap " ""<ESC>i
"inoremap { {}<ESC>i<CR><ESC>O

noremap \ :Ag<Space>

" Switch window
noremap <silent> <C-k> <C-W>k
noremap <silent> <C-j> <C-W>j
noremap <silent> <C-h> <C-W>h
noremap <silent> <C-l> <C-W>l

" NERDTree and Tagbar
nnoremap <F2> :NERDTreeToggle<CR>
noremap <C-n> :NERDTreeToggle<CR>
nnoremap <F4> :TagbarToggle<CR>

nnoremap <C-c> :q!<CR>
nnoremap <C-x> :wq!<CR>
nnoremap <C-d> :w<CR>

" Indent
""noremap <F8> gg=G
""inoremap <F8> <ESC>mzgg=G`z<Insert>

" Tab navigation
noremap <Leader>h :tabn<CR>
inoremap <Leader>h <esc>:tabn<CR><Insert>
noremap <Leader>l :tabprev<CR>
inoremap <Leader>l <ESC>tabprev<CR><Insert>
" }

" Automatically quit vim if NERDTree and tagbar are the last and only buffers
autocmd VimEnter * NERDTree
autocmd VimEnter * wincmd w
autocmd StdinReadPre * let s:std_in=1
autocmd VimEnter * if argc() == 0 && !exists("s:std_in") | NERDTree | endif
autocmd bufenter * if (winnr("$") == 1 && exists("b:NERDTree") && b:NERDTree.isTabTree()) | q | endif
autocmd bufenter * if (winnr("$") == 1 && exists("b:NERDTreeType") &&b:NERDTreeType == "primary") | q | endif


" disable py_lint on every write
""let g:pymode_lint_write = 0

" let tagbar to be compact
let g:tagbar_compact = 1

" Python customization {
function LoadPythonGoodies()

    if &ft=="python"||&ft=="html"||&ft=="xhtml"

        " set python path to vim, and virtualenv settings
        python << EOF
import os, sys, vim

for p in sys.path:
    if os.path.isdir(p):
        vim.command(r"set path+=%s" % (p.replace(" ", r"\ ")))

vir_env = os.environ.get('VIRTUAL_ENV', '')
if vir_env:
    act_this = os.path.join(vir_env, 'bin/activate_this.py')
    execfile(act_this, dict(__file__=act_this))
EOF

        " some nice adjustaments to show errors
        syn match pythonError "^\s*def\s\+\w\+(.*)\s*$" display
        syn match pythonError "^\s*class\s\+\w\+(.*)\s*$" display
        syn match pythonError "^\s*for\s.*[^:]\s*$" display
        syn match pythonError "^\s*except\s*$" display
        syn match pythonError "^\s*finally\s*$" display
        syn match pythonError "^\s*try\s*$" display
        syn match pythonError "^\s*else\s*$" display
        syn match pythonError "^\s*else\s*[^:].*" display
        "syn match pythonError "^\s*if\s.*[^\:]$" display
        syn match pythonError "^\s*except\s.*[^\:]$" display
        syn match pythonError "[;]$" display
        syn keyword pythonError         do

        let python_highlight_builtins = 1
        let python_highlight_exceptions = 1
        let python_highlight_string_formatting = 1
        let python_highlight_string_format = 1
        let python_highlight_string_templates = 1
        let python_highlight_indent_errors = 1
        let python_highlight_space_errors = 1
        let python_highlight_doctests = 1

        set ai tw=0 ts=4 sts=4 sw=4 et

    endif

endfunction

if !exists("myautocmds")
    let g:myautocmds=1

    "call LoadPythonGoodies()
    "autocmd Filetype python,html,xhtml call LoadPythonGoodies()
    au BufNewFile,BufRead *.py call LoadPythonGoodies()
    au BufRead,BufNewFile *.md set filetype=markdown

    " Omni completion
    autocmd FileType python set omnifunc=pythoncomplete#Complete
    autocmd FileType javascript set omnifunc=javascriptcomplete#CompleteJS
    autocmd FileType html set omnifunc=htmlcomplete#CompleteTags
    autocmd FileType css set omnifunc=csscomplete#CompleteCSS
    " Dissmiss PyDoc preview
    autocmd CursorMovedI * if pumvisible() == 0|pclose|endif
    autocmd InsertLeave * if pumvisible() == 0|pclose|endif

endif
"--ctags setting--
"    " 按下F5重新生成tag文件，并更新taglist
map <F5> :!ctags -R --c++-kinds=+p --fields=+iaS --extra=+q .<CR><CR>:TlistUpdate<CR>
imap <F5> <ESC>:!ctags -R --c++-kinds=+p --fields=+iaS --extra=+q .<CR><CR> :TlistUpdate<CR>
set tags=tags
set tags+=./tags "add current directory's generated tags file
set tags+=/home/hanyu/kaldi-trunk/src/tags "add new tags file(刚刚生成tags的路径，在ctags -R生成tags文件后，不要将tags移动到别的目录，否则ctrl+］时，会提示找不到源码文件)"
set tags+=/usr/include/c++/tags
"-- omnicppcomplete setting --
" 按下F3自动补全代码，注意该映射语句后不能有其他字符，包括tab；否则按下F3会自动补全一些乱码
imap <F3> <C-X><C-O>
" 按下F2根据头文件内关键字补全
imap <F2> <C-X><C-I>
set completeopt=menu,menuone " 关掉智能补全时的预览窗口
let OmniCpp_MayCompleteDot = 1 " autocomplete with .
let OmniCpp_MayCompleteArrow = 1 " autocomplete with ->
let OmniCpp_MayCompleteScope = 1 " autocomplete with ::
let OmniCpp_SelectFirstItem = 2 " select first item (but don't insert)
let OmniCpp_NamespaceSearch = 2 " search namespaces in this and included files
let OmniCpp_ShowPrototypeInAbbr = 1 " show function prototype in popup window
let OmniCpp_GlobalScopeSearch=1 " enable the global scope search
let OmniCpp_DisplayMode=1 " Class scope completion mode: always show all members
"let OmniCpp_DefaultNamespaces=["std"]
let OmniCpp_ShowScopeInAbbr=1 " show scope in abbreviation and remove the last column
let OmniCpp_ShowAccess=1 
"-- Taglist setting --
let Tlist_Ctags_Cmd='ctags' "因为我们放在环境变量里，所以可以直接执行
let Tlist_Use_Right_Window=1 "让窗口显示在右边，0的话就是显示在左边
let Tlist_Show_One_File=0 "让taglist可以同时展示多个文件的函数列表
let Tlist_File_Fold_Auto_Close=1 "非当前文件，函数列表折叠隐藏
let Tlist_Exit_OnlyWindow=1 "当taglist是最后一个分割窗口时，自动推出vim是否一直处理tags.1:处理;0:不处理
let Tlist_Process_File_Always=1 "实时更新tags
let Tlist_Inc_Winwidth=0
"-- WinManager setting --
let g:winManagerWindowLayout='FileExplorer|TagList' "
"    设置我们要管理的插件
let g:persistentBehaviour=0 " 如果所有编辑文件都关闭了，退出vim

" nerdcommenter configuration
" Add spaces after comment delimiters by default
let g:NERDSpaceDelims = 1
"
" Use compact syntax for prettified multi-line comments
let g:NERDCompactSexyComs = 1
"
" Align line-wise comment delimiters flush left instead of following code indentation
let g:NERDDefaultAlign = 'left'
"
" Set a language to use its alternate delimiters by default
let g:NERDAltDelims_java = 1
"
" Add your own custom formats or override the defaults
let g:NERDCustomDelimiters = { 'c': { 'left': '/**','right': '*/' }, 'cpp': { 'left': '//'} , 'h': {'left' : '//'}}
"
" Allow commenting and inverting empty lines (useful when commenting a region)
let g:NERDCommentEmptyLines = 1

" Enable trimming of trailing whitespace when uncommenting
let g:NERDTrimTrailingWhitespace = 1
let g:mapleader=","
nmap wm :WMToggle<cr>
