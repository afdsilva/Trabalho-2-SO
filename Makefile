# Trabalho SO 2 - Ferramenta para verificar sistema de arquivos FAT16
# André Felipe da Silva
# Matricula: 11107587
# e-mail: 	afdsilva@inf.ufpel.edu.br
# ultimo update: 10.02.2014

DATA=10.02.2014
TRABALHO=trabalho-so-2
EXECUTABLE=testeme
BASE_FILENAME=$(TRABALHO)\[$(DATA)\]
REVISION=1

#FLAGS=-lpthread
OBJECTS=

all: $(OBJECTS)
	@gcc -m32 exemplo.c -o $(EXECUTABLE)
	@echo "Programa compilado..."
criar_discos:
	@dd if=/dev/zero of=disco bs=512 count=32708
	@dd if=/dev/zero of=disco_2 bs=512 count=32708
	@mkfs.vfat -F16 disco
	@mkfs.vfat -F16 disco_2
	@mkdir -p ponto_montagem
	@mkdir -p ponto_montagem_disco_2
	
	@echo "Discos e Pontos de Montagem criados"
monta_particoes:
	@sudo mount -o loop,uid=1000,gid=1000 disco ponto_montagem/
	@sudo mount -o loop,uid=1000,gid=1000 disco ponto_montagem_disco_2/
	@echo "Particoes montadas"
desmonta_particoes:
	@sudo umount ponto_montagem/
	@sudo umount ponto_montagem_disco_2/
	@echo "Particoes desmontadas"
run:
	./$(EXECUTABLE)
clean:
ifneq ($(wildcard *.o),)
	rm *.o
endif
ifeq ($(wildcard $(EXECUTABLE)),$(EXECUTABLE))
	rm $(EXECUTABLE)
endif
