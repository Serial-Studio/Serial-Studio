# Style and format recursively
astyle --style=linux --indent=spaces --align-pointer=type --indent-preproc-block --indent-preproc-define --indent-col1-comments --pad-first-paren-out --pad-oper --attach-namespaces --remove-brackets --convert-tabs --close-templates --max-code-length=100 --max-instatement-indent=50 --lineend=windows --suffix=none --recursive ../../*.h ../../*.cpp ../../*.cc
