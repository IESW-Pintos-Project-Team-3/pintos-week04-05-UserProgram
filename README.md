# Pintos User Program 프로젝트: 사용자 프로그램 및 시스템 콜 구현

## 1. 프로젝트 개요

본 프로젝트는 Pintos 운영체제에서 **사용자 프로그램의 실행 환경을 구축하고, 운영체제와 사용자 프로그램 간의 상호작용을 위한 시스템 콜(System Call) 기능을 구현**하는 것을 목표로 합니다. 이 과정을 통해 **CPU, 메모리, 파일 관리 및 프로세스 제어와 관련된 운영체제의 핵심적인 자원 관리 방법**을 깊이 있게 이해하게 됩니다.

## 2. 주요 구현 목표

### 2.1 인자 파싱 및 전달 (Argument Parsing and Passing)

*   **`execute_thread` 함수 수정**: 프로그램 파일 이름뿐만 아니라, **프로그램에 전달될 인자들을 함께 처리**할 수 있도록 `execute_thread` 함수를 수정합니다.
*   **명령어 파싱**: 전달된 명령어를 `argc`와 `argv` 형태로 적절히 파싱해야 합니다. 이를 위해 `string.h`에 포함된 `strtok_r` 함수를 활용합니다.
*   **스택에 인자 전달**: 파싱된 인자들을 **Calling Convention에 맞춰 스택(Stack)에 전달**합니다. `if_.esp` (스택 포인터)를 사용하여 스택에 데이터를 올립니다.

### 2.2 시스템 콜 (파일 관리) 구현 (System Calls: File Management)

*   **파일 디스크립터 관리**:
    *   Pintos 커널이 파일에 대한 메타데이터(metadata)를 관리하는 방식을 이해하고, 파일 공유 및 I/O 리디렉션을 지원하도록 구현합니다.
    *   **프로세스당 파일 디스크립터 테이블을 관리**해야 하며, **0, 1, 2는 각각 표준 입력(STDIN_FILENO), 표준 출력(STDOUT_FILENO), 표준 에러(STDERR_FILENO)를 위해 예약**됩니다.
    *   `read` 시스템 콜은 `fd`가 `STDIN_FILENO` (0)인 경우 `input_getc()`를 사용하고, `write` 시스템 콜은 `fd`가 `STDOUT_FILENO` (1)인 경우 `putbuf()`를 사용합니다.
*   **실행 파일 쓰기 금지**: 파일이 실행될 때 `file_deny_write()` 함수를 통해 해당 파일에 대한 쓰기를 금지하고, 파일이 닫힐 때 `file_allow_write()` 함수를 통해 다시 쓰기가 가능하도록 구현합니다.
*   **9가지 파일 관리 시스템 콜 구현**:
    *   `bool create(const char *file, unsigned initial_size)`: 새 파일을 생성합니다.
    *   `int open(const char *file)`: 파일을 열고 파일 디스크립터를 반환합니다.
    *   `int read(int fd, void *buffer, unsigned size)`: `fd`에 해당하는 파일에서 데이터를 읽습니다.
    *   `int write(int fd, const void *buffer, unsigned size)`: `fd`에 해당하는 파일에 데이터를 씁니다.
    *   `void close(int fd)`: `fd`에 해당하는 파일을 닫습니다.
    *   `int filesize(int fd)`: `fd`에 해당하는 파일의 크기를 반환합니다.
    *   `void seek(int fd, unsigned position)`: `fd`의 파일 읽기/쓰기 위치를 `position`으로 이동합니다.
    *   `unsigned tell(int fd)`: `fd`의 현재 읽기/쓰기 위치를 반환합니다.
    *   `void remove(const char *file)`: 파일을 삭제합니다.

### 2.3 시스템 콜 (프로세스 관리) 구현 (System Calls: Process Management)

*   **프로세스 상태 및 관계 설계**:
    *   **프로세스 상태(Process State)**를 `TASK_STOPPED`, `TASK_RUNNING`, `TASK_READY`, `TASK_ZOMBIE` 등으로 설계하고 관리합니다.
    *   **프로세스 관계(Process Relationship)**를 `parent`, `children.next`, `children.prev`, `sibling.next`, `sibling.prev`와 같은 포인터를 사용하여 설계합니다.
*   **프로세스 종료 메시지 출력**: 사용자 프로그램의 `exit` 시스템 콜에 전달되는 인자(`status`)를 포함하여 프로세스 종료 메시지(예: `printf(“%s: exit(%d)\n”, …)`)를 출력합니다.
*   **4가지 프로세스 관리 시스템 콜 구현**:
    *   `void exit(int status)`: 현재 사용자 프로그램을 종료하고, `status`를 커널에 반환합니다. 이 과정에서 사용 중인 모든 자원을 해제하고, 프로세스 상태를 좀비(Zombie) 상태로 만들며, 모든 자식 프로세스의 부모를 `init` 프로세스로 지정하고 부모 프로세스를 깨웁니다.
    *   `pid_t exec(const char *cmd_line)`: `cmd_line`을 실행하여 새로운 프로세스를 생성하고 해당 프로세스의 `pid`를 반환합니다 (`fork()`와 유사). `exec` 함수는 부모 스레드에서 자식 스레드를 생성하며 프로세스 관계를 초기화합니다.
    *   `int wait(pid_t pid)`: `pid` 프로세스가 종료될 때까지 기다립니다. 자식이 없거나 커널에 의해 비정상 종료되면 -1을 반환합니다. 자식이 좀비 상태이면 해당 자식 엔트리를 해제하고 자식의 종료 상태를 반환합니다.
    *   `void halt(void)`: `power_off()`를 호출하여 시스템을 종료합니다.

## 3. Pintos 환경 설정 및 디스크 관리

프로젝트 진행을 위해 다음 명령어를 사용하여 Pintos 환경을 설정하고 디스크를 관리합니다:

*   **디스크 생성**: `pintos-mkdisk <img-file> <size>` (예: `pintos-mkdisk fs.dsk 2`).
*   **디스크 포맷**: `pintos -f -q` (`-f`는 포맷, `-q`는 부팅 후 종료).
*   **파일 디스크에 추가**: `pintos -p <source> -a <dest> -- -q` (예: `pintos –p ../../examples/shell –a shell -- -q`).
*   **프로그램 실행**: `pintos –q run ‘<executable>’` (예: `pintos –q run ‘shell’`).

## 4. 핵심 데이터 구조 및 내부 동작 이해

*   **시스템 콜 내부 구조**:
    *   사용자 프로그램이 `open()`과 같은 시스템 콜을 호출하면, `lib/user/syscall.c`의 래퍼(wrapper) 함수를 통해 실제 시스템 콜 번호와 인자들이 커널로 전달됩니다.
    *   커널의 `userprog/syscall.c`에 있는 `syscall_handler`는 인터럽트 프레임(`struct intr_frame`)을 통해 시스템 콜 번호를 확인하고 해당 커널 함수(`sys_open()`)를 호출합니다. 이 과정은 **IDT (Interrupt Descriptor Table) 및 `interrupt.c`를 통해 제어**됩니다.
*   **파일 관리 데이터 구조**:
    *   **커널은 파일에 대한 메타데이터(파일 속성, 디스크 블록 위치 등)를 유지**합니다. 파일 데이터는 디스크 섹터 단위로 비연속적으로 저장될 수 있습니다.
    *   `open()` 시스템 콜은 **디스크에서 파일의 메타데이터만 주 메모리로 가져오고, 파일 내용은 너무 클 수 있으므로 즉시 가져오지 않습니다**.
    *   **`inode` 구조체와 `file` 구조체를 분리하여 메타데이터 공유의 효율성을 높입니다**.
        *   **`inode` 구조체**: 여러 프로세스가 공유하는 파일 정보(예: 식별자, 보호, 크기, 위치)를 포함합니다.
        *   **`file` 구조체**: 프로세스별로 유지되는 파일 정보(예: 파일 오프셋 `pos`)를 포함합니다.
    *   **파일 디스크립터 테이블**: 각 프로세스는 고유한 디스크립터 테이블을 가지며, 이 테이블의 각 엔트리는 파일 테이블의 엔트리를 가리키고, 파일 테이블의 엔트리는 다시 `inode` 테이블의 엔트리를 가리킵니다. **모든 프로세스는 동일한 오픈 파일 및 `inode` 테이블을 공유**합니다. `struct thread`는 `u_open_files`를 통해 열린 파일 테이블을 관리합니다.
*   **프로세스 관리 데이터 구조**:
    *   프로세스 간의 관계는 `parent`, `children.next`, `children.prev`, `sibling.next`, `sibling.prev`와 같은 포인터를 통해 구현됩니다.
    *   `exec` 함수는 부모 스레드에서 자식 스레드를 생성하며 프로세스 관계를 초기화합니다.
    *   `wait` 함수는 자식 프로세스가 좀비 상태가 될 때까지 기다립니다.
    *   `exit` 함수는 사용 중인 자원을 해제하고 프로세스를 좀비 상태로 만듭니다.

## 5. 테스트 셋

구현 완료 후 다음 테스트 셋을 통해 기능의 정상 작동 여부를 확인할 수 있습니다:

*   **인자 전달 테스트**: `Arg-pass.c` (직접 구현 필요)
*   **프로세스 제어 테스트 (exec, wait, halt, exit)**: `Child-simple.c`, `Wait-simple.c`, `Wait-twice.c`
*   **파일 제어 시스템 콜 테스트 (open, close, read, write)**: `Open-bad-ptr.c`, `Open-normal.c`, `Read-bad-ptr.c`, `Read-normal.c`, `Write-bad-ptr.c`, `Write-normal.c`
*   **자체 개발 테스트**: `Self-developed.c` (직접 구현 필요)

---
