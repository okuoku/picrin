CONTRIB_LIBS += $(wildcard contrib/50.option/*.scm)
CONTRIB_TESTS += test-option

test-option: $(TEST_RUNNER)
	for test in `ls contrib/50.option/t/*.scm`; do \
	  ./$(TEST_RUNNER) "$$test"; \
	done
