# How to contribute as a developer

To contribute, fork this repository to create your own copy that you can clone locally and push back to. You can use your fork to create pull requests for your code to be merged into this repository.


## Contribution guidelines

Prefer opening fewer high quality Pull Requests over many low quality ones. Prioritize polishing existing Pull Requests instead of creating new ones. Do not open Pull Requests as draft unless there are specific requirements that justify it, because draft Pull Requests are generally not looked at and pollute the Pull Request section.


## AI code generation guidelines

Creating changes with LLM generated code is generally allowed. The author is responsible for verifying that all generated code is human readable, maintainable and logically correct. Furthermore, all generated code needs to be tested and verified. The author is not allowed to outsource the polishing of the generated code to the human code reviewers. In a Pull Request, generated code needs to be announced as such and to what extent it was polished by human intervention.

### New code contributors and AI generated code

New contributors are discouraged from submitting Pull Requests with thousands of lines changed or added with the help of LLMs, because human code reviewers cannot attend such volumes at the risk of wasting precious time with potentially poorly generated code.


## Code guidelines

### Scope of code changes

Code edits only touch the lines of code that serve the intended goal of the change. Big refactors should not be combined with logical changes, because these can become very difficult to review. If a change requires a refactor, create a commit for the refactor before (or after) creating a commit for the change. A Pull Request can contain multiple commits and can be merged with **Rebase and Merge** if these commits are meant to be preserved on the main branch. Otherwise, method of merging will be **Squash and Merge**.

### Style of code changes

Code edits should fit the nearby code in ways that the code style reads consistent, unless the original code style is bad. The original game code uses c++98, or a deviation thereof, and is simple to read. Prefer not to use newer language features unless required to implement the desired change. Prefer to use newer language features when they are considerably more robust or make the code easier to understand or maintain.
