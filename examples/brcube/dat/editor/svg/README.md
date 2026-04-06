```
for i in *.svg; do inkscape -w 64 -h 64 --export-filename ../"${i::-4}.png" "$i"; done
```
