#!/usr/bin/env python3

import json
import pathlib
import re
import subprocess
import sys
import threading

EXE = str(pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "bin/jeuCartes").resolve())
checks = 0


def run(*args, data="", expected=0):
    global checks
    result = subprocess.run([EXE, *args], input=data, text=True,
                            encoding="utf-8", capture_output=True, timeout=30)
    checks += 1
    if result.returncode != expected:
        raise AssertionError((args, result.returncode, result.stdout, result.stderr))
    return result


assert "JeuCartes" in run("--help").stdout
assert "1.0.0" in run("--version").stdout
run("--unknown", expected=1)
run("--game", expected=1)
run("--game", "unknown", expected=1)
run("--simulate", expected=1)
run("--game", "uno", "--players", "1", expected=1)
run("--game", "poker", "--players", "7", expected=1)
run("--game", "tarot", "--players", "3", expected=1)
run("--game", "uno", "--rounds", "0", expected=1)
run("--game", "uno", "--seed", "-1", expected=1)
run("--game", "uno", "--seed", "4294967296", expected=1)
run("--game", "uno", "--players", "3x", expected=1)
run("--game", "uno", "--humans", "2", expected=1)
run("--game", "uno", "--quiet", expected=1)
assert "JEUCARTES" in run(data="nonsense\n0\n").stdout
assert "interrompue" in run().stdout
assert "interrompue" in run(data="q\n").stdout
for game in ("uno", "familles", "poker", "tarot"):
    args = ("--game", game, "--simulate", "--seed", "42", "--rounds", "12", "--json")
    first = run(*args).stdout
    assert first == run(*args).stdout
    result = json.loads(first)
    assert result["game"] == game and 1 <= result["rounds"] <= 12
    assert len(result["scores"]) == 4 and result["actions"] > 0
    if game == "tarot":
        assert sum(result["scores"]) == 0
    # An interactive game must handle EOF and q at its first actual input.
    for data in ("", "q\n", "nonsense\nq\n"):
        output = run("--game", game, "--seed", "42", data=data).stdout
        assert "interrompue" in output
run("--game", "uno", "--simulate", "--seed", "4294967295", "--json")


def play_human(game, seed):
    """Play a complete console session through the actual public prompts."""
    global checks
    process = subprocess.Popen([EXE, "--game", game, "--seed", str(seed)],
                               stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT, text=True,
                               encoding="utf-8", bufsize=1)
    timer = threading.Timer(25, process.kill)
    timer.start()
    transcript, window = "", ""
    prompts = 0
    family, member = 1, 1
    members = ("Grand-pere", "Grand-mere", "Pere", "Mere", "Fils", "Fille")
    try:
        while True:
            char = process.stdout.read(1)
            if not char:
                break
            transcript += char
            window += char
            scalar = window.endswith("quitter] : ")
            multiple = window.endswith("q = quitter) : ")
            if not scalar and not multiple:
                continue
            prompts += 1
            if prompts > 5000:
                raise AssertionError(f"Too many interactive prompts ({game}): {transcript[-4000:]}")
            prompt = window.splitlines()[-1]
            if multiple:
                answer = "1 2"  # Actual draw exchange, not just stand pat.
            elif "A qui demander" in prompt:
                targets = re.findall(r"^(\d+) = Robot", window, re.MULTILINE)
                answer = targets[0]
                own = {}
                for fam, name in re.findall(r"^\s+\d+\. Famille (\d+) / (Grand-pere|Grand-mere|Pere|Mere|Fils|Fille)", window, re.MULTILINE):
                    own.setdefault(int(fam), set()).add(members.index(name) + 1)
                family = next(f for f in sorted(own) if len(own[f]) < 6)
                member = next(m for m in range(1, 7) if m not in own[family])
            elif prompt.startswith("Famille "):
                answer = str(family)
            elif prompt.startswith("Membre :"):
                answer = str(member)
            elif "Enchere :" in prompt:
                # Try a prise/garde to exercise taking the dog and discarding.
                current = re.search(r"enchere actuelle : ([^)]+)", prompt).group(1)
                answer = "1" if current == "Passe" else ("2" if current == "Prise" else "0")
            elif "chelem" in prompt or "Poignee" in prompt:
                answer = "0"
            elif prompt.startswith("Couleur :"):
                answer = "1"
            elif "Jouer cette carte" in prompt:
                answer = "1"
            elif "suivre/parole" in prompt:
                answer = "1"
            elif "Carte" in prompt:
                allowed = re.findall(r"^\s+(\d+)\. .*\*$", window, re.MULTILINE)
                answer = allowed[0] if allowed else "0"
            else:
                raise AssertionError(f"Unhandled prompt: {prompt}")
            process.stdin.write(answer + "\n")
            process.stdin.flush()
            window = ""
        process.wait(timeout=5)
        assert process.returncode == 0, transcript[-4000:]
        assert f"=== Resultat {game}" in transcript, transcript[-4000:]
        assert "interrompue" not in transcript and prompts > 0
        checks += 1
        return transcript
    finally:
        timer.cancel()
        if process.poll() is None:
            process.kill()
        process.wait()
        process.stdin.close()
        process.stdout.close()


for game in ("uno", "familles", "poker", "tarot"):
    play_human(game, 42)
# At least one test must enter the human taker's six-card discard phase.
tarot = play_human("tarot", 7)
assert "Carte a ecarter" in tarot
print(f"CLI : {checks} executions validees.")
