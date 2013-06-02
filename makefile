#Anno accademico        2012/2013
#Progetto               #1
#Giulio Fornasaro       151991
#Francesco Boschini     151840
#Luca Zamboni           151759
all : plive equal mkbkp

plive :
	@cd ./plive.d && make

equal :
	@cd ./equals.d && make

mkbkp :
	@cd ./mkbkp.d && make
