" Vim syntax file for SKIM

if exists("b:current_syntax")
  finish
endif

syn keyword skimReserved contained QUEUE CALLS NOUNS EXITS PLACE QUICK
syn keyword skimReserved QUEUE CALLS NOUNS EXITS PLACE QUICK 

syn match skimOperator contained containedin=skimBlock "\v\+="
syn match skimOperator "\v\@"
syn match skimOperator "\v\," contained containedin=skimNounBlock
syn match skimOperator "\v\-="
syn match skimOperator "\v\+"
syn match skimOperator "\v\-"
syn match skimOperator "\v\<\="
syn match skimOperator "\v\>\="
syn match skimOperator "\v\=" contained containedin=skimNounBlock
syn match skimOperator "\v\="
syn match skimOperator "\v,"
syn match skimLogic '|' 
syn match skimLogic '&' 

syn match skimNounDef "\v\[@<=[^<>\[\]\=]*" contained containedin=skimNounBlock
syn match skimVerbDef "\v\[@<=:([^\]]*)" contained containedin=skimNounBlock

syn match skimKeyword "\v[^<>\[\]]*" contained nextgroup=skimKeywordExecuteBlock
syn match skimKeywordExecute "\v([^<>\[\]]*)" contained

syn match skimValue  "\v\@([^\-\+\=\<\>\(\)\\\/\&\|\!\?\{\}\[\]])*" 
syn match skimNumber "\v\#([^\-\+\=\<\>\(\)\\\/\&\|\!\?\{\}\[\]])*" 
syn match skimNumber "\v\#(\d)+" contained containedin=skimNounBlock 

syn match skimInstruction "\v\!" 
syn match skimCondition "\v\?" 

syn match skimAsset "\v\$([^\/\n])*"

syn region skimNounBlock start="\v\[" end="\v\]"
 \ fold transparent oneline
 \ contains=skimReserved

syn region skimTextBlock start="\"" end="\"[cwd]\=" skip="\\\\\|\\\""
 \ fold  
 \ contains=skimFormat,skimKeywordBlock,skimText,skimFormat,skimCommentString
 \ nextgroup=skimFormat,skimText 

syn region skimKeywordBlock start="<" end=">"
 \ fold transparent contained oneline
 \ contains=skimKeywordExecuteBlock,skimKeyword,skimKeyStart,skimKeyEnd

syn region skimKeywordExecuteBLock start="\v\[" end="\v\]"
 \ fold contained transparent oneline
 \ contains=skimKeywordExecute,skimKeyBlockStart,skimKeyBlockEnd

syn match skimFormat "\v\\n" contained
syn match skimFormat "\v\\ " contained
syn match skimFormat "\v\*\*" contained
syn match skimFormat "\v\=\=" contained
syn match skimFormat "\v__" contained

syn match skimComment "\v//.*\n"
syn match skimCommentString "\v//.*\n" contained

syn sync minlines=100

let b:current_syntax = "skim"

" text and <keywords[inside]>
hi def link skimTextBlock String
hi def link skimFormat PreProc 
hi def link skimKeyword Identifier
hi def link skimKeywordExecute Type 

" // comments
hi def link skimComment Comment
hi def link skimCommentString Comment

" [noun] and [:verb] definitions
hi def link skimReserved PreProc
hi def link skimNounDef Type
hi def link skimVerbDef Type 

hi def link skimAsset Special

" evalutions with # and @
hi def link skimNumber Constant 
hi def link skimValue Constant 

" statements ! ? & |
hi def link skimInstruction Constant
hi def link skimCondition Conditional
hi def link skimLogic Conditional

hi def link skimOperator Special


