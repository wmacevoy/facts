#!/bin/sh
# The full check.  `make check` builds the test programs and the examples and
# then runs this; CI runs `make check`.  Success prints the last line below.
#
# Each case names the requirement it asserts (docs/REQUIREMENTS_v4.md).
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

  # REQ2/CK-3 exit code 1 when any fact was fiction; 2, and a line on stderr, when facts
  # were run and no fact was checked; 0 when the facts were not asked to run.
  exits 1 $t
  exits 2 $t --facts_include=Out
  says  "facts: no facts were checked" $t --facts_include=Out
  exits 2 $t --facts_include=NoSuchGroup
  says  "facts: no facts were checked" $t --facts_junit --facts_include=NoSuchGroup
  exits 0 $t --facts_skip
  exits 0 $tif

  # REQ2/CK-5 an argument beginning --fact is reserved: it must be an option facts knows,
  # or nothing runs.  Any other argument is the program's own and is left alone.
  exits 2 $t --facts_inclde=Out
  says  "facts: unknown option --facts_inclde=Out (--facts_help lists the options)" $t --facts_inclde=Out
  quiet "test/testfacts.$lang 3: Alpha facts check started" $t --facts_plain --facts_inclde=Out
  exits 2 $t --factor=3
  golden testfacts_${lang}_plain.out $t --facts_plain --an_option_of_the_program input.txt
  golden testfacts_if_facts_${lang}.out $tif --facts --an_option_of_the_program
  exits 2 $tif --facts --facts_inclde=Out
  says  "facts: unknown option --facts_inclde=Out (--facts_help lists the options)" $tif --facts --facts_inclde=Out

  # REQ2/SX-4 a timeout that is not a positive number of seconds is refused.
  exits 2 $t --facts_timeout=abc
  says  "facts: --facts_timeout needs a positive number of seconds, not 'abc'" $t --facts_timeout=abc
  exits 2 $t --facts_timeout=0

  # REQ1/RG-3 include starts from nothing and adds what matches; exclude removes what matches.
  says  "test/testfacts.$lang 17: Err facts check started"      $t --facts_plain '--facts_include=*Err'
  says  "test/testfacts.$lang 21: LargeErr facts check started" $t --facts_plain '--facts_include=*Err'
  says  "test/testfacts.$lang 3: Alpha facts check excluded."   $t --facts_plain '--facts_include=*Err'
  says  "test/testfacts.$lang 3: Alpha facts check excluded."   $t --facts_plain '--facts_exclude=Al*a'
  says  "test/testfacts.$lang 7: Integer facts check started"   $t --facts_plain '--facts_exclude=Al*a'
done

# REQ1/WR-5 in C++, FACT(a op b) prints both values, evaluates each side once, and
# FACT(a op b, note) prints its footnote only for a fiction, without evaluating it for a truth.
golden testfacts_expr_cpp_plain.out bin/testfacts_expr_cpp --facts_plain
says "test/testfacts_expr.cpp 58: ExprNoteLazy facts check ended" bin/testfacts_expr_cpp --facts_plain

# REQ1/WR-6 FACT(a,op,b) evaluates each side once in C (where the compiler can) and in C++;
# built the other way, a fiction evaluates twice and the report's last line says so.
golden testfacts_once_c_plain.out   bin/testfacts_once_c --facts_plain
golden testfacts_once_cpp_plain.out bin/testfacts_once_cpp --facts_plain
golden testfacts_twice_c_plain.out  bin/testfacts_twice_c --facts_plain
says  "test/testfacts_once.c/OnceWhenFiction 16: next() {=1} == 5 {=5} is fiction" bin/testfacts_once_c --facts_plain
says  "test/testfacts_once.c/OnceWhenFiction 16: next() {=1} == 5 {=5} is fiction" bin/testfacts_once_cpp --facts_plain
says  "warning: 2 of those fictions evaluated each side twice to print it; with this compiler, a side effect such as ++x in a FACT happens twice." bin/testfacts_twice_c --facts_plain
for once in bin/testfacts_once_c bin/testfacts_once_cpp bin/testfacts_c; do
  if ("$once" --facts_plain 2>&1 || true) | grep -q "^warning:"; then
    echo "FAIL: $once printed the evaluated-twice warning"; fail=1
  fi
done

# REQ1/RG-2 find registers the groups a source file declares, in the file's order,
# REQ1/RG-4 and --facts_register_all prints them as a block to paste.
found=$(bin/testfacts_c --facts_find test/find_subset.txt \; --facts_register_all 2>/dev/null | tr -d ' \n')
[ "$found" = "FACTS_REGISTER_ALL(){FACTS_REGISTER(Out);FACTS_REGISTER(Alpha);}" ] ||
  { echo "FAIL: --facts_find test/find_subset.txt did not register Out then Alpha, got: $found"; fail=1; }
# REQ1/WR-4 FactsAbsErr and FactsRelErr: nineteen facts about them, all truths.
exits 0 bin/testfacts_err_c
says  "19 (100.0%) truths and 0 (0.0%) fictions checked." bin/testfacts_err_c --facts_plain

# REQ3/WR-7 in C++, "x == interval" asks whether x is inside; pm, + and * build one around a
# value; a fiction prints the value and the interval with enough digits to see why.
golden testfacts_interval_cpp_plain.out bin/testfacts_interval_cpp --facts_plain
says  "test/testfacts_interval.cpp/OutsideAbsolute 81: x == 2.0 + pm(0.25) {2.5 == [1.75,2.25]} is fiction" bin/testfacts_interval_cpp --facts_plain

# REQ4/WR-9 a fiction shows any value: its own operator<<, a container's first 20 elements, a
# pair, or "?"; every comparison compiles.
golden testfacts_show_cpp_plain.out bin/testfacts_show_cpp --facts_plain
says  "test/testfacts_show.cpp/ShowOthers 40: x == y {? == ?} is fiction" bin/testfacts_show_cpp --facts_plain

# REQ4/WR-8 in C++ a fiction in a helper, a value-returning helper, or a lambda ends its group;
# FACT_BY_RETURN, and every fact in C, leaves only the function it is in; the next group runs.
leave=bin/testfacts_leave_cpp
golden testfacts_leave_cpp_plain.out $leave --facts_plain
exits 1 $leave
says  "reached: ReturnHelperGoesOn went on after its helper returned" $leave --facts_plain
says  "reached: CGroupHelperReturns went on after its helper returned" $leave --facts_plain
if ($leave --facts_plain 2>&1 || true) | grep -q '^not reached'; then
  echo "FAIL: $leave went on past a fiction that leaves its group"; fail=1
fi
# The same under --facts_unsafe: the run, not the child process, catches the fiction.
says  "7 (46.7%) truths and 8 (53.3%) fictions checked." $leave --facts_plain --facts_unsafe
# Built without exceptions, FACT leaves by return and the report is the same.
golden testfacts_cpp_plain.out bin/testfacts_noexc_cpp --facts_plain

# REQ1/RG-3 include narrows what find registered, whichever option comes first.
for order in "--facts_find test/testfacts.c ; --facts_include=Integer" "--facts_include=Integer --facts_find test/testfacts.c ;"; do
  says  "test/testfacts.c 7: Integer facts check started" bin/testfacts_c --facts_plain $order
  says  "test/testfacts.c 3: Alpha facts check excluded." bin/testfacts_c --facts_plain $order
  quiet "test/testfacts.c 3: Alpha facts check started"   bin/testfacts_c --facts_plain $order
done

# REQ1/RG-2, REQ1/RG-3 the same in a program with no registration block: find alone runs
# both groups; with an include, in either order, only the one named; with no find, nothing is
# known, and that is an error (REQ2/CK-3).
f=bin/testfacts_find_c
says  "2 (100.0%) truths and 0 (0.0%) fictions checked." $f --facts_plain --facts_find test/testfacts_find.c
exits 0 $f --facts_find test/testfacts_find.c
for order in "--facts_find test/testfacts_find.c ; --facts_include=FoundSecond" "--facts_include=FoundSecond --facts_find test/testfacts_find.c"; do
  says  "test/testfacts_find.c 9: FoundSecond facts check started" $f --facts_plain $order
  says  "test/testfacts_find.c 5: FoundFirst facts check excluded." $f --facts_plain $order
  says  "1 (100.0%) truths and 0 (0.0%) fictions checked." $f --facts_plain $order
done
says  "test/testfacts_find.c 5: FoundFirst facts check excluded." $f --facts_plain --facts_exclude=FoundFirst --facts_find test/testfacts_find.c
exits 2 $f

# REQ1/RG-2 a declared group that is not in the executable is named, and the rest still register.
says  "facts: AboutInts not found in executable" bin/testfacts_c --facts_find test/sample_facts.c test/find_subset.txt \; --facts_skip
quiet "facts: Out not found in executable"       bin/testfacts_c --facts_find test/sample_facts.c test/find_subset.txt \; --facts_skip

# REQ2/CK-3 a run whose facts were all truths exits 0.
exits 0 bin/testfacts_once_cpp --facts_include=OnceWhenTruth

# REQ1/RG-2 a group name too long to look up is named, not dropped.
printf 'FACTS(%0300d) {\n' 0 > bin/long_name.txt
says "facts: a group name in bin/long_name.txt is over 255 characters" bin/testfacts_c --facts_find bin/long_name.txt \; --facts_skip

# REQ1/SX-1 a group that crashes does not take the caller down: stderr names the signal,
# REQ2/CK-3 and the exit code is 2.  Without the crash the same program exits 0.
bad=bin/testfacts_bad_c
exits 0 $bad
exits 2 $bad --facts_include=Crashes
says  "facts: child killed by signal 11 (SIGSEGV)" $bad --facts_include=Crashes
exits 2 $bad --facts_include=Aborts
says  "facts: child killed by signal 6 (SIGABRT)" $bad --facts_include=Aborts
# REQ1/SX-2 a run still going at its timeout is killed, asleep or busy, and stderr says so.
exits 2 $bad --facts_timeout=1 --facts_include=Sleeps
says  "facts: TIMEOUT after 1 seconds" $bad --facts_timeout=1 --facts_include=Sleeps
exits 2 $bad --facts_timeout=1 --facts_include=Spins
says  "facts: TIMEOUT after 1 seconds" $bad --facts_timeout=1 --facts_include=Spins
# REQ3/SX-5 a signal sent to the parent alone (kill, a stop button, a cancelled job) also ends
# the run it was supervising: the child is gone, and the parent ends by that same signal.
# SIGINT takes the same path but cannot be tried here: a shell starts a background job with
# SIGINT ignored, and facts leaves an ignored signal ignored.
for sig in TERM HUP; do
  pidfile=bin/sleeps.pid; rm -f $pidfile
  FACTS_TEST_PIDFILE=$pidfile $bad --facts_timeout=60 --facts_include=Sleeps >/dev/null 2>&1 &
  parent=$!
  tries=0
  until [ -s $pidfile ] || [ $tries -ge 50 ]; do tries=$((tries + 1)); sleep 0.1 2>/dev/null || sleep 1; done
  child=$(cat $pidfile 2>/dev/null)
  kill -$sig $parent
  wait $parent; code=$?
  case $sig in TERM) want=143;; HUP) want=129;; esac
  [ "$code" = "$want" ] || { echo "FAIL: parent sent SIG$sig exited $code, expected $want"; fail=1; }
  if [ -n "$child" ] && kill -0 "$child" 2>/dev/null; then
    echo "FAIL: after SIG$sig to the parent, the child $child is still running"; fail=1
    kill -KILL "$child" 2>/dev/null
  fi
  [ -n "$child" ] || { echo "FAIL: the child never wrote $pidfile"; fail=1; }
done

# REQ1/SX-2 safe execution does not wait when there is nothing to wait for: twenty short runs
# in well under the twenty seconds they took when the parent looked once a second.
start=$(date +%s)
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do $bad >/dev/null 2>&1; done
took=$(( $(date +%s) - start ))
[ "$took" -lt 5 ] || { echo "FAIL: twenty safe runs took $took seconds, expected under 5"; fail=1; }

# REQ1/SHAPE-3 each foreign model's words stay inside its edge of src/facts.c (a veto, not the verdict).
sh test/borders.sh src/facts.c || { echo "FAIL: test/borders.sh"; fail=1; }

if [ "$fail" = 0 ]; then
  echo "facts: all checks passed"
else
  echo "facts: CHECK FAILED"
  exit 1
fi
