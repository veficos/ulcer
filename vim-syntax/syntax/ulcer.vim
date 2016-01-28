" Vim syntax file
" Language:     ulcer
" Maintainer:   Brian Mock <mock.brian@gmail.com>
"
" Based on syntax/ulcer.vim from Vim 7.3

if !exists("main_syntax")
  if version < 600
    syntax clear
  elseif exists("b:current_syntax")
    finish
  endif
  let main_syntax = 'ulcer'
endif

" Drop fold if it set but vim doesn't support it.
if version < 600 && exists("ulcer_fold")
  unlet ulcer_fold
endif

setlocal iskeyword+=$

syn match   ulcerOps               '+\|++\|-\|--\|\*\|/\|>\|<\|>>\|<<\|>>>\||\|&\|||\|&&\|%\|\~\|!\|=\|?\|:\|+=\|-=\|\*=\|/='
syn match   ulcerDot               '\.'
syn match   ulcerComma             ','
syn match   ulcerSemicolon         ';'

syn keyword ulcerCommentTodo      TODO FIXME XXX TBD contained
syn match   ulcerLineComment      "\/\/.*" contains=@Spell,ulcerCommentTodo
syn match   ulcerCommentSkip      "^[ \t]*\*\($\|[ \t]\+\)"
syn region  ulcerComment          start="/\*"  end="\*/" contains=@Spell,ulcerCommentTodo
syn match   ulcerSpecial          "\\\d\d\d\|\\."
syn region  ulcerStringD          start=+"+  skip=+\\\\\|\\"+  end=+"\|$+  contains=ulcerSpecial,@htmlPreproc
syn region  ulcerStringS          start=+'+  skip=+\\\\\|\\'+  end=+'\|$+  contains=ulcerSpecial,@htmlPreproc

syn match   ulcerSpecialCharacter "'\\.'"
" syn match   ulcerNumber           '-\=\<\d\+L\=\>\|0[xX][0-9a-fA-F]\+\>\|NaN'
syn match   ulcerNumber           '-\=\<\d\+\>\|0[xX][0-9a-fA-F]\+\>'
syn match   ulcerFloat            '-\=\<\d\+\.\d\+\([eE]\d\+\)\=\>'
syn match   ulcerFloat            '-\=\<\d\+[eE]\d\+\>'
syn region  ulcerRegexpString     start=+/[^/*]+me=e-1 skip=+\\\\\|\\/+ end=+/[gi]\{0,2\}\s*$+ end=+/[gi]\{0,2\}\s*[;.,)\]}]+me=e-1 contains=@htmlPreproc oneline

syn keyword ulcerConditional       if else switch
syn keyword ulcerRepeat            while for do in
syn keyword ulcerBranch            break continue
syn keyword ulcerOperator          new delete instanceof typeof
syn keyword ulcerType              Array Boolean Date Function Number Object String RegExp Math
syn keyword ulcerStatement         return with var let
syn keyword ulcerBoolean           true false
syn keyword ulcerNaN               NaN
syn keyword ulcerNull              null undefined
syn keyword ulcerIdentifier        arguments this
syn keyword ulcerLabel             case default
syn keyword ulcerException         try catch finally throw
syn keyword ulcerMessage           alert confirm prompt status
syn keyword ulcerGlobal            self window top parent
syn keyword ulcerMember            document event location
syn keyword ulcerDeprecated        escape unescape
syn keyword ulcerReserved          abstract boolean byte char class const debugger double enum export extends final float goto implements import int interface long native package private protected public short static super synchronized throws transient volatile

" if exists("ulcer_fold")
"     syn match   ulcerFunction      "\<function\>"
"     syn region  ulcerFunctionFold  start="\<function\>.*[^};]$" end="^\z1}.*$" transparent fold keepend

"     syn sync match ulcerSync       grouphere ulcerFunctionFold "\<function\>"
"     syn sync match ulcerSync       grouphere NONE "^}"

"     setlocal foldmethod=syntax
"     setlocal foldtext=getline(v:foldstart)
" else
"     syn keyword ulcerFunction      function
"     syn match   ulcerBraces           "[{}\[\]]"
"     syn match   ulcerParens           "[()]"
" endif

syn keyword ulcerFunction      function
syn match   ulcerBraces           "[{}\[\]]"
syn match   ulcerParens           "[()]"

syn sync fromstart
syn sync maxlines=100

if main_syntax == "ulcer"
  syn sync ccomment ulcerComment
endif

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_ulcer_syn_inits")
  if version < 508
    let did_ulcer_syn_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif
  HiLink ulcerComment              Comment
  HiLink ulcerLineComment          Comment
  HiLink ulcerCommentTodo          Todo
  HiLink ulcerSpecial              Special
  HiLink ulcerStringS              String
  HiLink ulcerStringD              String
  HiLink ulcerCharacter            Character
  HiLink ulcerSpecialCharacter     ulcerSpecial
  HiLink ulcerNumber               Number
  HiLink ulcerFloat                Float
  HiLink ulcerConditional          Conditional
  HiLink ulcerRepeat               Repeat
  HiLink ulcerBranch               Conditional
  HiLink ulcerOperator             Operator
  HiLink ulcerOps                  Operator
  HiLink ulcerStatement            Statement
  HiLink ulcerFunction             Function
  HiLink ulcerError                Error
  HiLink javaScrParenError              ulcerError
  HiLink ulcerNull                 Keyword
  HiLink ulcerBoolean              Boolean
  HiLink ulcerRegexpString         String

  HiLink ulcerNaN                  Number

  HiLink ulcerDot                  Dot
  HiLink ulcerType                 Type
  HiLink ulcerBraces               Braces
  HiLink ulcerParens               Parens
  HiLink ulcerComma                Comma
  HiLink ulcerSemicolon            Semicolon

  HiLink Dot                            Punctuation
  HiLink Type                           Punctuation
  HiLink Braces                         Punctuation
  HiLink Parens                         Punctuation
  HiLink Comma                          Punctuation
  HiLink Semicolon                      Punctuation

  HiLink ulcerIdentifier           Identifier
  HiLink ulcerLabel                Label
  HiLink ulcerException            Exception
  HiLink ulcerMessage              Keyword
  HiLink ulcerGlobal               Identifier
  HiLink ulcerMember               Identifier
  HiLink ulcerDeprecated           Exception
  HiLink ulcerReserved             Keyword
  HiLink ulcerDebug                Debug
  HiLink ulcerConstant             Label

  delcommand HiLink
endif

let b:current_syntax = "ulcer"
if main_syntax == 'ulcer'
  unlet main_syntax
endif