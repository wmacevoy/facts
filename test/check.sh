#!/bin/sh
# The full check.  `make check` builds the test programs and the examples and
# then runs this; CI runs `make check`.  Success prints the last line below.
#
# Each case names the requirement it asserts (docs/REQUIREMENTS_v1.md).
# The golden files in expected/ are the published console and JUnit formats:
# refresh them with `make expected` only when a format change is intended.

fail=0

# golden FILE COMMAND...: the command's stdout is exactly expected/FILE.
golden() {
  file=$1; shift
  ("$@" 2>/dev/null || true) | diff - "expected/$file" ||
    { echo "FAIL: $* differs from expected/$file"; fail=1; }
}

# exits CODE COMMAND...: the command's exit code is CODE.
exits() {
  want=$1; shift
  "$@" >/dev/null 2>&1; got=$?
  [ "$got" = "$want" ] || { echo "FAIL: $* exited $got, expected $want"; fail=1; }
}

# says TEXT COMMAND...: the command's stdout or stderr has a line that is exactly TEXT.
says() {
  text=$1; shift
  ("$@" 2>&1 || true) | grep -qxF -- "$text" ||
    { echo "FAIL: $* did not print: $text"; fail=1; }
}

# quiet TEXT COMMAND...: no line of the command's output is exactly TEXT.
quiet() {
  text=$1; shift
  if ("$@" 2>&1 || true) | grep -qxF -- "$text"; then
    echo "FAIL: $* printed: $text"; fail=1
  fi
}

for lang in c cpp; do
  t=bin/testfacts_$lang
  tif=bin/testfacts_if_$lang
  hello=examples/hello_$lang/hello

  # REQ1/WR-2 a fiction prints where, the expressions, and the values, and ends its group.
  # REQ1/CK-1 the console report: started, ended, ended badly, then the summary and counts.
  # REQ1/RG-1 groups run in the order they were registered.
  golden testfacts_${lang}.out        $t
  # REQ1/CK-2 --facts_plain prints the same report with no color codes.
  golden testfacts_${lang}_plain.out  $t --facts_plain
  # REQ1/JU-1 one case per registered group, its output captured and escaped inside it.
  golden testfacts_${lang}_junit.out  $t --facts_junit
  # REQ1/CK-4 FACTS_MAIN_IF runs the program's own main without the switch, the facts with it.
  golden testfacts_if_${lang}.out       $tif
  golden testfacts_if_facts_${lang}.out $tif --facts
  # REQ1/WR-1 the examples build from hello.*, facts.h, facts.c (and facts.cpp) alone.
  # REQ1/WR-3 (hello_cpp) a watch prints a gdb breakpoint line carrying the values.
  golden hello_${lang}.out        $hello
  golden hello_${lang}_plain.out  $hello --facts_plain
  golden hello_${lang}_junit.out  $hello --facts_junit

  # REQ1/SX-3 --facts_unsafe runs in this process and reports the same.
  golden testfacts_${lang}_plain.out  $t --facts_unsafe --facts_plain

  # REQ1/CK-3 exit code 1 when any fact was fiction, 0 when none was.
  exits 1 $t
  exits 0 $t --facts_include=Out
  exits 0 $tif

  # REQ1/RG-3 include starts from nothing and adds what matches; exclude removes what matches.
  says  "test/testfacts.$lang 17: Err facts check started"      $t --facts_plain '--facts_include=*Err'
  says  "test/testfacts.$lang 21: LargeErr facts check started" $t --facts_plain '--facts_include=*Err'
  says  "test/testfacts.$lang 3: Alpha facts check excluded."   $t --facts_plain '--facts_include=*Err'
  says  "test/testfacts.$lang 3: Alpha facts check excluded."   $t --facts_plain '--facts_exclude=Al*a'
  says  "test/testfacts.$lang 7: Integer facts check started"   $t --facts_plain '--facts_exclude=Al*a'
done

# REQ1/RG-2 find registers the groups a source file declares, in the file's order,
# REQ1/RG-4 and --facts_register_all prints them as a block to paste.
found=$(bin/testfacts_c --facts_find test/find_subset.txt \; --facts_register_all 2>/dev/null | tr -d ' \n')
[ "$found" = "FACTS_REGISTER_ALL(){FACTS_REGISTER(Out);FACTS_REGISTER(Alpha);}" ] ||
  { echo "FAIL: --facts_find test/find_subset.txt did not register Out then Alpha, got: $found"; fail=1; }
# REQ1/RG-2 a declared group that is not in the executable is named, and the rest still register.
says  "facts: AboutInts not found in executable" bin/testfacts_c --facts_find test/sample_facts.c test/find_subset.txt \; --facts_skip
quiet "facts: Out not found in executable"       bin/testfacts_c --facts_find test/sample_facts.c test/find_subset.txt \; --facts_skip

# REQ1/SHAPE-3 each foreign model's words stay inside its edge of src/facts.c (a veto, not the verdict).
sh test/borders.sh src/facts.c || { echo "FAIL: test/borders.sh"; fail=1; }

if [ "$fail" = 0 ]; then
  echo "facts: all checks passed"
else
  echo "facts: CHECK FAILED"
  exit 1
fi
