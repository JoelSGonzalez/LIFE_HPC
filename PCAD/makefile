# Compiler and flags
CC = gcc
CFLAGS = -fopenmp -Wall
LDFLAGS = -g

# Source files
SOURCE = baseline.c

OUTPUT = baseline.exe

compile:
	$(CC) $(CFLAGS) $(SOURCE) -o $(OUTPUT) $(LDFLAGS)

execute:
	./$(OUTPUT) -s 2000 -g 1000

# Listas de Parâmetros
THREADS_LIST  := 1 2 4 8 10 16 20 32 40
BIND_LIST     := false true close spread master
PLACES_LIST   := cores threads sockets
SCHEDULE_LIST := static dynamic

# Parâmetros Padrão/Default para Destaque
DEFAULT_THREADS  := 1
DEFAULT_BIND     := false
DEFAULT_PLACES   := cores
DEFAULT_SCHEDULE := static

# ------------------------------------------------------------------
# template: Define a estrutura de cada comando independente
# Argumentos: $(1)=BIND, $(2)=PLACES, $(3)=SCHEDULE
# ------------------------------------------------------------------
define run_template
.PHONY: run_$(1)_$(2)_$(3)
run_$(1)_$(2)_$(3): $$(OUTPUT)
	> run_$(1)_$(2)_$(3).txt;
	@for t in $$(THREADS_LIST); do \
		t_lbl="$$$$t"; [ "$$$$t" = "$(DEFAULT_THREADS)" ] && t_lbl="$$$$t (DEFAULT)"; \
		b_lbl="$(1)"; [ "$(1)" = "$(DEFAULT_BIND)" ] && b_lbl="$(1) (DEFAULT)"; \
		p_lbl="$(2)"; [ "$(2)" = "$(DEFAULT_PLACES)" ] && p_lbl="$(2) (DEFAULT)"; \
		s_lbl="$(3)"; [ "$(3)" = "$(DEFAULT_SCHEDULE)" ] && s_lbl="$(3) (DEFAULT)"; \
		echo "--------------------------------------------------"; \
		echo "Testing Matrix Configuration:"; \
		echo "  OMP_NUM_THREADS = $$$$t_lbl"; \
		echo "  OMP_PROC_BIND   = $$$$b_lbl"; \
		echo "  OMP_PLACES      = $$$$p_lbl"; \
		echo "  OMP_SCHEDULE    = $$$$s_lbl"; \
		echo "--------------------------------------------------"; \
		OMP_NUM_THREADS=$$$$t \
		OMP_PROC_BIND=$(1) \
		OMP_PLACES=$(2) \
		OMP_SCHEDULE=$(3) \
		./$$(OUTPUT) -s 50 -g 50 >> run_$(1)_$(2)_$(3).txt; \
		echo >> run_$(1)_$(2)_$(3).txt;\
	done
endef

# ------------------------------------------------------------------
# Loops do GNU Make para gerar dinamicamente todos os alvos individuais
# ------------------------------------------------------------------

# OMP_PROC_BIND=false → apenas OMP_PLACES=cores
$(foreach s,$(SCHEDULE_LIST),\
    $(eval $(call run_template,false,cores,$(s)))\
)

# Demais valores de OMP_PROC_BIND → todas as opções de OMP_PLACES
$(foreach b,true close spread master,\
    $(foreach p,$(PLACES_LIST),\
        $(foreach s,$(SCHEDULE_LIST),\
            $(eval $(call run_template,$(b),$(p),$(s)))\
        )\
    )\
)

# Alvo para executar toda a bateria de testes
run_all_battery: \
    $(foreach s,$(SCHEDULE_LIST),run_false_cores_$(s)) \
    $(foreach b,true close spread master,\
        $(foreach p,$(PLACES_LIST),\
            $(foreach s,$(SCHEDULE_LIST),run_$(b)_$(p)_$(s))))

clean:
	rm -f $(OBJS) $(OUTPUT)
