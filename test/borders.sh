#!/bin/sh
# The border check for src/facts.c (REQ1/SHAPE-3).
#
# facts.c is one file on purpose (REQ1/WR-1), so its parts are sections, each
# opened by a line "// ==== border: NAME ====": four edges, then the core.
# This check keeps a foreign model's words inside its own edge.  A word search
# cannot see a call from one section into another, so it may veto, and the
# holder of the shape judges the rest.
#
# Usage: sh test/borders.sh [file]     (default src/facts.c)

awk '
function only(owner, words) {
  if ($0 ~ words && border != owner) {
    printf "%s:%d: belongs in border %s, found in %s: %s\n", FILENAME, FNR, owner, (border == "" ? "(none)" : border), $0
    bad = 1
  }
}
/^\/\/ ==== border: / {
  border = $4
  if (border in seen) { printf "%s:%d: border %s opens twice; each part is one section\n", FILENAME, FNR, border; bad = 1 }
  seen[border] = 1
  next
}
{
  # Each foreign model is spelled at one edge.
  only("edge-junit",      "testsuite|testcase|system-out|system-err|<skipped|<failure")
  only("edge-os-process", "fork\\(|execv\\(|waitpid\\(|sigaction\\(|sigsuspend\\(|sigprocmask\\(|WIFSIGNALED|WTERMSIG|SIGKILL|SIGCHLD|SIGALRM|SIGTERM|SIGHUP|CreateProcess|TerminateProcess|WaitForSingleObject")
  only("edge-os-symbols", "dlsym\\(|GetProcAddress\\(")
  only("edge-gdb",        "\\(gdb\\)")
}
END {
  n = split("edge-gdb edge-junit edge-os-symbols edge-os-process core", want, " ")
  for (i = 1; i <= n; ++i) if (!(want[i] in seen)) { printf "%s: no border named %s\n", FILENAME, want[i]; bad = 1 }
  exit bad
}' "${1:-src/facts.c}"
