# CommTester — Development Rules

## 1. GitHub is the single source of truth

The GitHub repository is the authoritative source for the current project state and working code.

After a significant technical decision, the relevant project files must be updated in GitHub. Chat discussion alone does not constitute a project change.

## 2. Do not create documentation for its own sake

Do not create documentation files unless they materially simplify development, testing, maintenance, or recovery of project context.

Prefer keeping the project documentation compact and useful.

## 3. Record significant decisions immediately

A significant technical decision must be recorded when it is accepted.

Example:

`DECISION: Arduino Nano is the platform for the compact portable Modbus/RS-232 line tester.`

## 4. Explicitly classify project information

Do not turn assumptions into facts. Use the following status labels:

- `FACT` — verified by test, measurement, source code, documentation, or another reliable source.
- `DECISION` — explicitly accepted for the project.
- `ASSUMPTION` — currently believed to be true but not yet sufficiently verified.
- `TBD` — unresolved or unknown.
- `REJECTED` — considered and explicitly rejected.

## 5. Search for existing solutions before implementing new ones

Before developing a custom function or subsystem, check existing open-source projects, libraries, protocol implementations, reference designs, and relevant industrial implementations.

A custom implementation is preferred only when there is a clear technical reason, such as resource usage, required behavior, architecture, licensing, reliability, or lack of suitable functionality.

## 6. Develop by Milestones

Development should proceed sequentially through defined Milestones. Do not move to the next major stage until the current stage satisfies its completion criteria.

Current high-level CommTester path:

- `M0` — Hardware base
- `M1` — Modbus RTU Slave
- `M2` — Modbus RTU Master
- `M3` — Bidirectional line testing
- `M4` — Diagnostics and statistics
- `M5` — Display
- `M6` — Portable enclosure
- `M7` — Final tester

## 7. Every task must have a completion criterion

Tasks must be expressed as objectively testable outcomes rather than vague goals.

Bad:

`Check Modbus Master.`

Good:

`Master sends a valid FC03 request, receives the response, validates CRC, parses the requested registers, and reports timeout and invalid-response conditions correctly.`

## 8. Prove before designing around an assumption

When behavior is unknown and can reasonably be tested, prefer an experiment or measurement over architectural speculation.

Record the result as a `FACT` and only then use it as a basis for a `DECISION`.

## 9. Change one major layer at a time

Avoid simultaneously changing multiple independent subsystems. When practical, complete and test one layer before modifying the next.

This keeps failures attributable to a specific change and preserves a reliable baseline.

## 10. Maintain explicit project state at significant checkpoints

At the end of each significant stage, update the project state with:

- `CURRENT STATE` — what exists and works now.
- `COMPLETED` — what has been finished and verified.
- `DECISIONS` — accepted technical decisions.
- `FACTS` — important verified observations.
- `OPEN QUESTIONS` — unresolved issues.
- `NEXT STEP` — one immediate development task.
- `BLOCKERS` — issues preventing progress.

Checkpoint entries are intended to preserve a recoverable development baseline and prevent loss of project context.

## 11. Preserve working baselines

A tested working state must be preserved before substantial experimental changes. Experimental work should be isolated where practical and must not silently replace a known-good baseline.

## 12. Project scope

CommTester is the compact portable Modbus/RS-232 line tester being developed on Arduino Nano. It is a practical test platform and development aid for the larger diagnostic instrument planned separately.

The compact tester and the future main diagnostic instrument must not be conflated when making architectural decisions.
