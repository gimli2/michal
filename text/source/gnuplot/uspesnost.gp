reset
set encoding iso_8859_2

set output "uspesnost.pdf"

set terminal pdf interlace enhanced
unset multiplot

set title 'Úspěšnost měření mikrotvrdosti'

set xlabel 'Přesnost měření'
set xrange[75:101]
set format x '%2.0f%%'

set ylabel 'Počet naměřených dat'

set key title "Legenda"
#set key outside center bottom
set key left
set key box width 1 heigh 1 

#set autoscale

plot "uspesnost.txt" using 1:3 title "Originální SW" axes x1y1 with linespoints, \
"" using 1:2 title "Program MICHAL" axes x1y1 with linespoints, \
"" using 1:4 title "Tato práce" axes x1y1 with linespoints

