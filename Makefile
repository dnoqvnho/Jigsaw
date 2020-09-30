raw_data := data_generator/raw_data
load_baseline := data_loader/baselines
load_jigsaw := data_loader/HV
partitioner_row_v := partitioner/Row-V
partitioner_column_h := partitioner/Column-H
partitioner_jigsaw := partitioner/HV
processor := query_processor
clear_cache := clear_cache

.PHONY: all clean $(raw_data) $(load_baseline) $(partitioner_row_v) \
		$(partitioner_column_h) $(partitioner_jigsaw) $(load_jigsaw) \
		$(processor) $(clear_cache)

all: $(raw_data) $(load_baseline) $(partitioner_row_v) \
		$(partitioner_column_h) $(partitioner_jigsaw) $(load_jigsaw) \
		$(processor) $(clear_cache)

clean:
	rm -f ./bins/*
	$(MAKE) --directory=$(raw_data) clean
	$(MAKE) --directory=$(load_baseline) clean
	$(MAKE) --directory=$(partitioner_row_v) clean
	$(MAKE) --directory=$(partitioner_column_h) clean
	$(MAKE) --directory=$(partitioner_jigsaw) clean
	$(MAKE) --directory=$(load_jigsaw) clean
	$(MAKE) --directory=$(clear_cache) clean
	rm -rf $(processor)

$(raw_data):
	mkdir -p ./bins/
	$(MAKE) --directory=$@
	mv $@/data_generator ./bins/raw_data

$(load_baseline):
	$(MAKE) --directory=$@
	mv $@/load/main ./bins/load_baseline

$(load_jigsaw):
	$(MAKE) --directory=$@
	mv $@/main ./bins/load_jigsaw

$(partitioner_row_v):
	$(MAKE) --directory=$@
	mv $@/main ./bins/partitioner_row_v

$(partitioner_column_h):
	$(MAKE) --directory=$@
	mv $@/main ./bins/partitioner_column_h

$(partitioner_jigsaw):
	$(MAKE) --directory=$@
	mv $@/main ./bins/partitioner_jigsaw

$(clear_cache):
	$(MAKE) --directory=$@
	mv $@/main ./bins/clear_cache


$(processor):
	./install-processor.sh
