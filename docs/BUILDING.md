## Code Formatting with clang-format

To ensure consistent code formatting, use `clang-format`.

### Installation

- On macOS:
  ```
  $ brew install clang-format
  ```

- On Linux:
  ```
  $ sudo apt install clang-format-19
  ```

- On Windows:
  ```
  $ pip install clang-format~=19.0
  ```

### Running clang-format

To format all files in the repository, run the following command:

  ```
  ninja -C build clang-format
  ```

---
