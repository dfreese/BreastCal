 SPACE=`du -h --max-depth=0 | awk '{print $1}'`
rm ./Tech/*dat.root
rm *_R.panel.root
rm *_L.panel.root
rm ./L?/*_L?.cal.root
rm ./R?/*_R?.cal.root
ENDSPACE=`du -h --max-depth=0 | awk '{print $1}'`
echo "Initial sixe:: $SPACE. Folder size after cleanup :: $ENDSPACE" 