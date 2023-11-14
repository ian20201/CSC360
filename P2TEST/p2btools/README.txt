p2bscore calculates your score from your eval file. Usage:

	./p2bscore path/to/eval



p2btest performs the function described in p2bfeedback.txt. Usage:

	(cd check; make)  # Build the check program.
	./p2btest path/to/p2.tar.gz

If you want to execute p2btest outside of the directory that contains it, you
may need to edit the 3 shell variables at the top of p2btest.



Internally, p2btest uses simplify (which simplifies the output of mts into a
more machine-friendly form) and check (which checks that a given file
containing simplified mts output is a valid output for a given input file). In
case you want to test your submission against your own tests, you can use
simplify and check independently of p2btest like this:

	mts test.txt > raw.txt
	simplify raw.txt | tee out.txt
	# Make sure `simplify` doesn't say "ERROR" anywhere. If it did, your
	# output format is incorrect.
	check test.txt out.txt && echo "PASS" || echo "FAIL"
