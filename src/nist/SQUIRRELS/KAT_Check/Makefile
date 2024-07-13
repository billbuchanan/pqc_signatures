REF_DIR=../Reference_Implementation
SQ_I_DIR=$(REF_DIR)/squirrels-I
SQ_II_DIR=$(REF_DIR)/squirrels-II
SQ_III_DIR=$(REF_DIR)/squirrels-III
SQ_IV_DIR=$(REF_DIR)/squirrels-IV
SQ_V_DIR=$(REF_DIR)/squirrels-V

SQ_I_KAT=KAT-Squirrels-I-12829872.rsp
SQ_II_KAT=KAT-Squirrels-II-16258752.rsp
SQ_III_KAT=KAT-Squirrels-III-29053632.rsp
SQ_IV_KAT=KAT-Squirrels-IV-35418288.rsp
SQ_V_KAT=KAT-Squirrels-V-50725632.rsp

.PHONY: all check clean check_I check_II check_III check_IV check_V

all: $(SQ_I_KAT) $(SQ_II_KAT) $(SQ_III_KAT) $(SQ_IV_KAT) $(SQ_V_KAT)

%katint:
	make -C $(shell dirname $*) build lib build/katint

$(SQ_I_KAT): $(SQ_I_DIR)/build/katint
	$<
$(SQ_II_KAT): $(SQ_II_DIR)/build/katint
	$<
$(SQ_III_KAT): $(SQ_III_DIR)/build/katint
	$<
$(SQ_IV_KAT): $(SQ_IV_DIR)/build/katint
	$<
$(SQ_V_KAT): $(SQ_V_DIR)/build/katint
	$<

check_I: $(SQ_I_KAT)
	sha256sum --check $(<:.rsp=.sha256)
check_II: $(SQ_II_KAT)
	sha256sum --check $(<:.rsp=.sha256)
check_III: $(SQ_III_KAT)
	sha256sum --check $(<:.rsp=.sha256)
check_IV: $(SQ_IV_KAT)
	sha256sum --check $(<:.rsp=.sha256)
check_V: $(SQ_V_KAT)
	sha256sum --check $(<:.rsp=.sha256)
check: check_I check_II check_III check_IV check_V

clean:
	rm -rf *.req *.rsp
