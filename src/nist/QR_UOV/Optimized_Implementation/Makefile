# ref portable64 avx2 avx512
platform := portable64
# subdirs := qruov1q127L3v156m54 
subdirs :=\
  qruov1q7L10v740m100  qruov1q31L3v165m60  qruov1q31L10v600m70   qruov1q127L3v156m54  \
  qruov3q7L10v1100m140 qruov3q31L3v246m87  qruov3q31L10v890m100  qruov3q127L3v228m78  \
  qruov5q7L10v1490m190 qruov5q31L3v324m114 qruov5q31L10v1120m120 qruov5q127L3v306m105

.PHONY: all clean $(subdirs)

all: $(subdirs)

$(subdirs): qruov_config.src
	mkdir -p $@/$(platform)
	grep $@ qruov_config.src > $@/$(platform)/qruov_config.txt
	sh -c "cd $@/$(platform) ; ln -s ../../$(platform)/* . || true"
	$(MAKE) -C $@/$(platform)

clean:
	rm -rf $(subdirs)
