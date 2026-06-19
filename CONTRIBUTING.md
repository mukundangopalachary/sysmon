# Contributing to Sysmon

First off, thank you for considering contributing to Sysmon! Contributions from everyone are welcome, whether it's reporting a bug, proposing a new feature, writing documentation, or submitting a Pull Request.

## How Can I Contribute?

### 1. Reporting Bugs
If you find a bug, please open an Issue on GitHub. Include:
- Your operating system and kernel version (`uname -r`).
- The exact steps to reproduce the issue.
- What you expected to happen vs what actually happened.
- Any crash logs or terminal output.

### 2. Suggesting Enhancements
Have an idea for a new dashboard panel or an optimization? 
- Open an Issue with the tag `enhancement`.
- Describe the feature in detail.
- Provide examples of how it would look or operate.

### 3. Writing Code (Pull Requests)
1. **Fork** the repository and clone it to your local machine.
2. **Create a new branch** for your feature or bug fix:
   ```bash
   git checkout -b feature/my-new-panel
   ```
3. **Write your code**. Ensure you follow the project's existing coding standards:
   - Use `C11` for the core engine (`src/core/`).
   - Use `C++17` for the UI framework (`src/tui/`).
   - Do not use blocking operations in the `TUI` thread! All `/proc` reading must happen in the `Core` engine.
4. **Test your code**. Run `make build` and ensure the application runs smoothly without memory leaks.
5. **Commit your changes**. Use clear, corporate-standard commit messages.
6. **Push** to your fork and submit a **Pull Request** to the `master` branch.

## Plugin Contributions
If you've written an awesome Bash plugin (like a sensor monitor or a custom Docker stats scraper) and want to share it, please submit it via a PR to the `scripts/plugins/` directory. Ensure your script outputs a maximum of 16 lines of clean text without infinite loops.

Thank you for making Sysmon better!
