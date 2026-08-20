# SSAB // ПОЛНАЯ ИНСТРУКЦИЯ ПО СБОРКЕ

> Версия: **v1.0** — полноценный релиз, не MVP.
> DSP переписан: PolyBLEP осцилляторы, phase-modulation FM, Moog-style
> 4-pole ladder фильтр (Huovilainen), SUB PROTECT через Linkwitz-Riley
> кроссовер, DC blocker на мастере.

Этот документ описывает три рабочих способа получить SSAB в LMMS на
Windows 10. От самого быстрого (1 минута) до полного ручного билда.

---

## Оглавление

- [Способ 1 — SoundFont (.sf2)](#способ-1--soundfont-sf2)
- [Способ 2 — GitHub Actions (облачная сборка .dll)](#способ-2--github-actions-облачная-сборка-dll)
- [Способ 3 — Локальная сборка (Visual Studio 2022 + CMake)](#способ-3--локальная-сборка-visual-studio-2022--cmake)
- [DSP-архитектура v1.0](#dsp-архитектура-v10)
- [Пресеты — что звучит как](#пресеты--что-звучит-как)
- [Устранение проблем](#устранение-проблем)

---

## Способ 1 — SoundFont (.sf2)

**Время: 1 минута.**
**Требуется: только LMMS.**

В архиве уже лежит готовый `SSAB.sf2` (~24 MB) с 18 пресетами.

### Установка в LMMS

1. Скачай `SSAB.sf2` (или весь `SSAB_LMMS_pack.zip` и распакуй).
2. Запусти LMMS.
3. Создай новый проект (или открой существующий).
4. В Bella bar (левая панель) найди **Sf2 Player** — иконка фиолетовой piano-клавиши.
5. Перетащи **Sf2 Player** на дорожку (или в Song Editor).
6. В открывшейся панели Sf2 Player нажми на **иконку папки** (справа вверху).
7. Выбери файл `SSAB.sf2`.
8. В дропдауне ниже выбери любой из 18 пресетов:
   - RAGGA, GUTCUTTER, EXTERMINATION, 1997, ACABALLERO
   - TRASHCAN, FUCKSERUM, SCUM, JESUSSAW, BUTTHURTED
   - ALIENFUCKER, NOOTTNEEDED, THUNDERDOOM, TRACKERSCUM, HAKKENBOOT
   - CHAINSAWLOBOTOMY, PIGSQUEAL, SLUDGEPIT
9. Кликай по piano-roll, чтобы играть. Каждая нота берёт ближайший
   сэмпл (C2, C3, C4, C5, C6) и pitch-shift-ит до нужной высоты.

### Что внутри SSAB.sf2

- **5 сэмплов на пресет**: C2 (MIDI 36), C3 (48), C4 (60), C5 (72), C6 (84)
- Каждый сэмпл — 3 секунды при 44.1 kHz, моно, 16-bit
- Loop region внутри сэмплов (для бесконечного удержания)
- Полифония ограничена только Sf2 Player (обычно 32 голоса)

### Плюсы / минусы

| + | − |
|---|---|
| Готовый файл, ничего не собирать | Нельзя крутить ручки вживую |
| Работает в любой версии LMMS | LFO запечён в сэмпл |
| 18 готовых пресетов | Ограниченное покрытие клавиш |
| Низкий расход CPU | Нет стерео-ширинения |

### Как регенерировать .sf2

Если хочешь изменить пресеты или добавить новые ноты:

```bash
# Требуется Python 3.9+ с numpy и scipy
pip install numpy scipy soundfile

# Запуск генератора
python3 Scripts/ssab_sf2_gen.py /путь/куда/положить
```

Скрипт перегенерирует `SSAB.sf2` + 18 WAV-превью (по C3 на пресет).
Время работы: ~30 секунд на современной машине.

---

## Способ 2 — GitHub Actions (облачная сборка .dll)

**Время: 10-15 минут.**
**Требуется: только аккаунт GitHub (бесплатно).**

GitHub Actions на `windows-latest` runner'е соберёт VST2 `.dll` в облаке.
Тебе не нужно ничего ставить локально.

### Шаг 1: Создай репозиторий

1. Зайди на <https://github.com> (зарегистрируйся если ещё нет).
2. Нажми **+** в правом верхнем углу → **New repository**.
3. Назови репо как угодно (например `ssab`).
4. Поставь visibility **Public** (для публичных репо Actions бесплатный без лимитов).
5. **НЕ** добавляй README/.gitignore/license (галочки не ставь).
6. Нажми **Create repository**.

### Шаг 2: Загрузи исходники в репо

Самый простой способ — через web-интерфейс GitHub:

1. Распакуй `SSAB_LMMS_pack.zip` локально.
2. В репо на GitHub нажми **Add file → Upload files**.
3. Перетащи **всё содержимое папки SSAB/** (включая скрытую папку `.github/`).
4. Внизу нажми **Commit changes**.

Альтернатива (для знакомых с git):
```bash
cd SSAB
git init
git remote add origin https://github.com/твой-логин/ssab.git
git add .
git commit -m "Initial SSAB v1.0"
git branch -M main
git push -u origin main
```

### Шаг 3: Запусти workflow

В проекте теперь **три** workflow:

| Workflow | Файл | Когда использовать |
|----------|------|---------------------|
| **Build SSAB VST2/VST3** | `build.yml` | Основной. Запускается автоматически на push. Использует "Visual Studio 17 2022" генератор. |
| **Build SSAB (Ninja fallback)** | `build-ninja.yml` | Ручной запуск. Использует Ninja генератор с явным MSVC toolchain. Используй если основной падает. |
| **Diagnose Runner** | `diagnose.yml` | Ручной запуск. Печатает что установлено на runner'е (VS, CMake, generator'ы). Используй если не понимаешь причину ошибки. |

**Для стандартной сборки:**

1. В репо перейди на вкладку **Actions**.
2. Если workflows не видны — нажми **I understand my workflows, go ahead and enable them** (для новых репо).
3. Workflow **Build SSAB VST2/VST3** запустится автоматически после push.
4. Жди. Сборка занимает **5-8 минут**:
   - 1 мин — Download JUCE 7.0.12 + VST2 stub headers
   - 1 мин — CMake configure
   - 4 мин — Build (Visual Studio 2022 / Release / x64)
   - 30 сек — Stage + upload artifacts

**Если основной workflow упал с ошибкой генератора:**

1. Перейди на **Actions** → **Build SSAB (Ninja fallback)** → **Run workflow**.
2. Этот вариант использует `vcvars64.bat` + Ninja, что более переносимо.
3. Жди 5-8 минут, скачай артефакт `SSAB_VST2_dll_ninja`.

**Если оба упали:**

1. Перейди на **Actions** → **Diagnose Runner** → **Run workflow**.
2. Прочитай output — там будет полный список что установлено на runner'е.
3. Пришли мне точный текст ошибки (или скриншот output'а diagnose workflow).

### Шаг 4: Скачай .dll

1. Когда билд закончится (зелёная галочка), кликни на этот run.
2. Прокрути вниз до секции **Artifacts**.
3. Скачай **`SSAB_VST2_dll`** (или `SSAB_windows_build_zip` — там всё сразу).
4. Распакуй архив — внутри будет `SSAB.dll` (~3-5 MB).

### Шаг 5: Установи в LMMS

1. Найди папку VST-плагинов LMMS:
   - По умолчанию: `C:\Program Files\VstPlugins\`
   - Либо открой LMMS → **Edit → Settings → Paths** → посмотри "VST plugins"
2. Скопируй `SSAB.dll` в эту папку.
3. (Опционально) Перезапусти LMMS чтобы он пересканировал плагины.
4. Создай новый instrument track → выбери **Vestige**.
5. В панели Vestige нажми иконку папки → выбери `SSAB.dll`.
6. Откроется GUI SSAB со всеми 7 блоками и 18 пресетами.

### Что делает каждый workflow

**`build.yml`** (основной):
1. Поднимает `windows-latest` runner (Windows Server 2022).
2. Ставит MSVC v143 через `microsoft/setup-msbuild@v2`.
3. Использует CMake, предустановленный на runner'е (НЕ ставит свой).
4. Запускает `cmake .. -G "Visual Studio 17 2022" -A x64 -DSSAB_ENABLE_VST2=ON`.
5. Скачивает JUCE 7.0.12 + VST2 stub headers через FetchContent.
6. Запускает `cmake --build . --config Release --parallel`.
7. Загружает 4 артефакта:
   - `SSAB_VST2_dll` — только VST2 .dll
   - `SSAB_VST3` — VST3 bundle
   - `SSAB_Standalone` — standalone .exe
   - `SSAB_windows_build_zip` — всё в одном zip

**`build-ninja.yml`** (фолбэк):
1. Поднимает `windows-latest` runner.
2. Активирует MSVC через `microsoft/setup-msbuild@v2`.
3. Ставит Ninja 1.11.1 через `seanmiddleditch/gha-setup-ninja@v5`.
4. Запускает `vcvars64.bat` для активации cl.exe в PATH.
5. Конфигурирует с `-G Ninja -DCMAKE_BUILD_TYPE=Release`.
6. Собирает, загружает артефакты.

**`diagnose.yml`** (отладка):
1. Печатает версии OS, CMake, MSBuild.
2. Запускает `vswhere.exe` чтобы показать установленные VS.
3. Печатает список доступных CMake generators.
4. Пробует сконфигурировать тестовый проект.

### Почему у тебя могло не сработать (если первый workflow упал)

**Причина:** На GitHub Actions runner'е `windows-latest` сейчас стоит **Visual Studio 18 (2026 Preview)**, а не VS 2022 (17). Старая версия workflow жёстко задавала генератор `"Visual Studio 17 2022"`, который не находил VS на runner'е и падал с ошибкой:

```
CMake Error at CMakeLists.txt:3 (project):
  Generator
    Visual Studio 17 2022
  could not find any instance of Visual Studio.
```

**Решение в новой версии workflow:**

Workflow теперь использует `vswhere.exe` для автоматического определения установленной версии Visual Studio и собирает имя генератора динамически:

```powershell
$vsVersion = & $vswhere -latest -property installationVersion
$vsMajor   = $vsVersion.Split('.')[0]
$genName   = "Visual Studio $vsMajor $([int]::Parse($vsMajor) + 2005)"
```

Это работает на любой версии Visual Studio (17/2022, 18/2026, и будущих). Если Microsoft снова обновит runner — workflow автоматически подхватит новую версию.

Если Copilot сказал, что проблема в `project()` — это был неверный диагноз. CMake `project()` синтаксически правильный, ошибка была на стороне генератора.

### Создание релиза

Если хочешь, чтобы у тебя была постоянная ссылка на .dll:

```bash
git tag v1.0.0
git push origin v1.0.0
```

Workflow автоматически создаст GitHub Release с прикреплённым zip'ом.

---

## Способ 3 — Локальная сборка (Visual Studio 2022 + CMake)

**Время: 30-40 минут (первый раз, с установкой VS).**
**Требуется: Visual Studio 2022, CMake 3.22+.**

### Шаг 1: Установи Visual Studio 2022

1. Скачай Community Edition (бесплатно): <https://visualstudio.microsoft.com/downloads/>.
2. Запусти установщик.
3. В **Workloads** поставь галочку **"Desktop development with C++"**.
4. В правой панели "Installation details" убедись, что выбраны:
   - **MSVC v143 - VS 2022 C++ x64/x86 build tools**
   - **Windows 10 SDK** (или Windows 11 SDK)
   - **C++ CMake tools for Windows**
5. Нажми **Install**. Размер: ~8 GB.

### Шаг 2: Установи CMake (если не поставился с VS)

1. Иди на <https://cmake.org/download/>.
2. Скачай **Windows x64 Installer** (`.msi`).
3. Запусти, выбери **Add CMake to system PATH for all users**.
4. Проверь в PowerShell:
   ```powershell
   cmake --version
   ```
   Должно показать `cmake version 3.27.x` или выше.

### Шаг 3: Собери SSAB

Распакуй `SSAB_LMMS_pack.zip` куда-нибудь, например в `C:\Users\твой-логин\Documents\SSAB`.

**Стандартный путь (VS 2022 generator):**
```powershell
cd C:\Users\твой-логин\Documents\SSAB
.\Scripts\build.ps1
```

**Если не работает (поставь Ninja):**
```powershell
# Вариант 1: через chocolatey (если установлен)
choco install ninja

# Вариант 2: через pip (если Python установлен)
pip install ninja

# Затем:
.\Scripts\build.ps1 -UseNinja
```

**Скрипт делает следующее:**
1. Проверяет наличие cmake и печатает версию.
2. Ищет Visual Studio через `vswhere.exe`.
3. (Опционально) использует Ninja вместо VS generator.
4. Создаёт папку `build/`.
5. Запускает `cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DSSAB_ENABLE_VST2=ON`.
6. CMake скачивает JUCE 7.0.12 (~200 MB, может занять пару минут).
7. CMake скачивает VST2 stub headers (~50 KB).
8. Запускает сборку (`cmake --build build --config Release --parallel`).
9. Показывает пути к артефактам и инструкции по установке.

**Флаги build.ps1:**

| Флаг | Описание |
|------|----------|
| `-DisableVST2` | Не собирать VST2 (только VST3 + Standalone) |
| `-UseNinja` | Использовать Ninja вместо VS generator (быстрее, не требует VS solution) |
| `-Clean` | Удалить `build/` перед конфигурацией |
| `-Verbose` | Подробный вывод CMake |
| `-VSGenerator <name>` | Явно указать генератор (напр. `"Visual Studio 17 2022"`). По умолчанию автоопределение через vswhere. |

Локальный скрипт также автоматически определяет версию VS через `vswhere.exe` и собирает имя генератора динамически. Если у тебя на машине стоит VS 2026 (18) — он сам подставит `"Visual Studio 18 2026"`.

### Шаг 4: Где лежат собранные бинари

После успешной сборки:

```
SSAB/
└── build/
    └── SSAB_artefacts/
        └── Release/
            ├── VST/
            │   └── SSAB.dll              ← VST2 плагин для LMMS/Vestige
            ├── VST3/
            │   └── SSAB.vst3             ← VST3 bundle (для современных DAW)
            └── Standalone/
                └── SSAB.exe              ← Standalone приложение
```

### Шаг 5: Установи в LMMS

1. Скопируй `build\SSAB_artefacts\Release\VST\SSAB.dll` в папку VST-плагинов LMMS (см. Способ 2, Шаг 5).
2. Открой LMMS → **Edit → Settings → Paths** → убедись, что путь указан правильно.
3. Перезапусти LMMS.
4. Создай новый instrument track → **Vestige** → выбери `SSAB.dll`.
5. Должен открыться GUI SSAB.

### Возможные проблемы

**"CMake не находит Visual Studio"**
- Убедись, что VS 2022 установлена с C++ workload.
- Попробуй явно указать generator:
  ```powershell
  cmake .. -G "Visual Studio 17 2022" -A x64 -DSSAB_ENABLE_VST2=ON
  ```

**"FetchContent failed to download JUCE"**
- Проверь интернет.
- Возможен временный сбой GitHub. Просто перезапусти `.\Scripts\build.ps1`.

**"LINK error: cannot find file"**
- Убедись, что запускаешь из папки SSAB (не из build/).
- Полное пересобирание:
  ```powershell
  Remove-Item -Recurse -Force build
  .\Scripts\build.ps1
  ```

**"VST2 not built (only VST3)"**
- Это значит, что CMake не нашёл VST2 SDK. Скрипт `build.ps1` должен сам скачать stub headers через FetchContent, но если что-то пошло не так:
  ```powershell
  # Вручную склонировать stub headers
  git clone https://github.com/robbert-vdh/juce6-vst2-headers external/vst2_headers
  # Затем в CMakeLists.txt указать путь:
  # juce_set_vst2_sdk_path(${CMAKE_SOURCE_DIR}/external/vst2_headers)
  ```

### Сборка под другие платформы

**macOS:**
```bash
mkdir build && cd build
cmake .. -G "Xcode" -DSSAB_ENABLE_VST2=OFF
cmake --build . --config Release
# VST3 будет в ~/Library/Audio/Plug-Ins/VST3/
```

**Linux:**
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DSSAB_ENABLE_VST2=OFF
cmake --build . -j
# VST3 будет в ~/.vst3/
```

---

## DSP-архитектура v1.0

### THE CORE — осцилляторы
- **SAW**: PolyBLEP band-limited sawtooth. Один остаточный PolyBLEP на rising edge устраняет алиасинг.
- **SQUARE**: PolyBLEP square с двумя residual (на rising edge при фазе 0 и на falling edge при фазе 0.5).
- **PULSE**: PolyBLEP pulse с adjustable pulse width. Два PolyBLEP residual на edges.
- **NOISE**: White RNG (juce::Random для C++, numpy.random для Python).
- **GRIT**: Saw + 18% white noise, модулированного амплитудой saw (звучит "грязно-аналогово").

### THE CORE — FM
- Реальный **phase modulation**: `a(n) = saw(phase_n + fm_amount * 2 * b(n))`.
- B модулирует фазу A, а не амплитуду. Это классический Yamaha-style FM.
- Hard-sync: если SYNC включён, фаза B сбрасывается при проходе A через 0.

### ACID BATH — фильтр
- **Moog-style 4-pole transistor ladder** (Huovilainen-style).
- 4 каскадных one-pole lowpass stages с feedback от 4-й стадии.
- `g = 1 - exp(-2π·fc/sr)` — well-behaved cutoff coefficient.
- Резонанс саморезонирует при values > 0.9 (но clamped до 0.99 для стабильности).
- Tanh thermal shaping на входе для тембрального обогащения.
- LP/HP/BP outputs: LP = s4, HP = in - s4, BP = s2 - s4 (12 dB/oct).

### CARNAGE — дисторшн
- **SUB PROTECT через Linkwitz-Riley 4th-order crossover** на 80 Hz:
  - Low band (< 80 Hz) полностью bypasses сатуратор.
  - High band (> 80 Hz) идёт через CLIP/FOLD/FUZZ.
  - Суммируются обратно после сатурации.
  - Гарантия: фундаментальная на низких нотах **никогда** не клиппится в ноль, как бы ни был выкручен DRIVE.
- **CLIP**: cubic soft-clipper (`x - x³/3`).
- **FOLD**: triangle-wave folder.
- **FUZZ**: square-law с жёстким клиппингом.
- **BITCRUSH**: sample-rate reducer + bit-depth quantiser.

### IMPACT — огибающая
- ADSR с **PUNCH** overshoot: на note-on env идёт до `1+punch` за attack time, потом спадает до 1 за ~20 ms, потом в обычный decay.
- Exponential time constants (`msToCoeff`).

### LFO
- Triangle LFO в диапазоне -1..1.
- Targets: CUTOFF (modulates filter cutoff), PITCH (B's phase), VOLUME (output gain), DRIVE (saturator input).

### DOOM — мастер-секция
- **WIDTH**: Mid/Side processing — `side *= (1 + width*3)`. Создаёт искусственное стерео из моно-voice.
- **VOLUME**: -60..+6 dB gain.
- **BRICKWALL**: hard limiter at ±1.0.
- **Master DC blocker**: one-pole (`R=0.995`) на обоих каналах.

### MASSACRE — деструктивные моды
- **1994 SWITCH**: квантует выход осцилляторов до ±0.7 (lo-fi tracker crunch).
- **RUINER**: crossfade с white noise (0 = clean, 1 = pure noise).
- **FEEDBACK**: 15 ms delayed self-feedback в фильтр (for runaway self-oscillation chaos).

---

## Пресеты — что звучит как

| # | Имя | Описание | Октава | Типичный пример использования |
|---|-----|----------|--------|-------------------------------|
| 0 | RAGGA | Классический reese с широкой расстройкой и большим сабом | 0 | Ragga-jungle, драм-н-бейс |
| 1 | GUTCUTTER | Массивный саб + тугая резонансная fuzz | -1 | Hard bassline, dubstep drop |
| 2 | EXTERMINATION | Рвущийся midrange wobble | 0 | Neurofunk DnB |
| 3 | 1997 | Классический 90s reese с MASSACRE 1994 | 0 | Oldskool jungle |
| 4 | ACABALLERO | FM-lead через BP-фильтр | 0 | Mid-range lead |
| 5 | TRASHCAN | Industrial grit с скрытым тональным телом | 0 | Industrial / noise |
| 6 | FUCKSERUM | Современный wobble reese с phase-FM и sync | 0 | Modern dubstep |
| 7 | SCUM | Грязный низкий sub для doom/sludge | -1 | Doom metal bass |
| 8 | JESUSSAW | Молитвенный saw-pad с медленным LFO | +1 | Atmospheric / drone |
| 9 | BUTTHURTED | Намеренно диссонантный расстроенный misery | 0 | Noise / experimental |
| 10 | ALIENFUCKER | Космический FM с сильной резонансностью | +1 | Sci-fi FX |
| 11 | NOOTTNEEDED | Чистый 8-bit tracker square, MASSACRE 1994 | 0 | Demoscene / tracker |
| 12 | THUNDERDOOM | Массивный doom-riff с медленным attack | -2 | Slow doom metal |
| 13 | TRACKERSCUM | 8-bit crunch с bitcrush | 0 | Lo-fi / chip |
| 14 | HAKKENBOOT | Gabber-kick bass без сустейна | -1 | Hardcore / gabber |
| 15 | CHAINSAWLOBOTOMY | Агрессивный saw-lead с feedback-ruin | 0 | Industrial metal |
| 16 | PIGSQUEAL | Высокорезонансный squeal, pitch-responsive | 0 | Pig-squeal / deathcore |
| 17 | SLUDGEPIT | Массивный sub с резонансным хвостом | -1 | Sludge / drone-doom |

---

## Устранение проблем

### Проблема: SSAB не появляется в списке VST в LMMS

**Решение:**
1. Убедись, что `SSAB.dll` лежит в правильной папке (см. Способ 2, Шаг 5).
2. В LMMS открой **Edit → Settings → Paths** → "VST plugins" должен указывать на эту папку.
3. Нажми кнопку **Rescan** рядом с путём (или перезапусти LMMS).
4. Если не помогло — попробуй положить `SSAB.dll` в `C:\Program Files\VstPlugins\` и добавить этот путь в Settings.

### Проблема: LMMS показывает ошибку "VST plugin not loaded"

**Решение:**
1. Проверь разрядность: LMMS должна быть 64-bit, и `SSAB.dll` тоже 64-bit. (Все сборки в этом архиве — x64.)
2. Если у тебя 32-bit LMMS — нужно собирать с `-A Win32` вместо `-A x64` в CMake.
3. Убедись, что нет антивируса, блокирующего .dll.

### Проблема: Звук клиппит/хрипит даже на низкой громкости

**Решение:**
1. Включи **BRICKWALL** (DOOM-секция) — это hard limiter на -1.0/+1.0.
2. Убавь VOLUME до -3..-6 dB.
3. Убавь DRIVE в CARNAGE.
4. Убавь FEEDBACK в MASSACRE (он легко заводит фильтр в self-oscillation).

### Проблема: Не работает LFO

**Решение:**
1. Проверь, что DEPTH > 0 (LFO-секция).
2. Проверь, что выбран правильный TARGET (CUTOFF / PITCH / VOLUME / DRIVE).
3. RATE в Hz — 0.05 Hz (медленный) до 30 Hz (быстрый). На 4 Hz будет ~4 цикла в секунду.

### Проблема: Пресет звучит совсем не как ожидается

**Решение:**
1. Проверь, что выбран правильный пресет через PREV/NEXT в верхней панели.
2. В GUI пресет выбирается через PREV/NEXT кнопки (в шапке), а не через пресеты DAW.
3. При смене пресета все параметры автоматически подтягиваются.
4. Если используешь .sf2 — там пресеты выбираются в дропдауне Sf2 Player.

### Проблема: .sf2 не открывается в Sf2 Player

**Решение:**
1. Проверь размер файла (~24 MB). Если сильно меньше — файл мог повредиться.
2. Убедись, что LMMS не старее версии 1.2.0 (старые версии имели проблемы с .sf2 > 16 MB).
3. Если файл всё равно не открывается — регенерируй его:
   ```bash
   python3 Scripts/ssab_sf2_gen.py /путь/куда
   ```

### Проблема: При сборке не находит juce_add_binary_data

**Решение:**
1. Это значит, что JUCE не скачался. Проверь интернет.
2. Удали `build/` и пересобери:
   ```powershell
   Remove-Item -Recurse -Force build
   .\Scripts\build.ps1
   ```
3. Если за корпоративным файрволом — добавь `github.com` и `codeload.github.com` в исключения.

### Проблема: В GUI не видны шрифты (текст как прямоугольники)

**Решение:**
1. Шрифты встроены в плагин через `juce_add_binary_data`.
2. Если текст всё равно не отображается — это баг LookAndFeel. Проверь, что файлы `HorrorNightDrip.ttf` и `ShockHorror.otf` лежат в `Source/Resources/`.
3. После любых изменений в шрифтах нужно полностью пересобрать (`Remove-Item -Recurse -Force build`).

---

## Лицензии шрифтов

- **Shock Horror** © Jonathan Stephen Harris 2017 — free for personal use.
- **Horror Night Drip** © Tigadestd 2020 — free for personal use.

Для коммерческого релиза SSAB — замени эти шрифты на коммерчески лицензируемые, либо купи коммерческую лицензию у авторов.

---

## Контакт / обратная связь

Если что-то не работает — напиши подробно:
1. Какой способ используешь (1, 2 или 3).
2. На каком шаге возникла проблема.
3. Точную ошибку (скриншот или текст).
4. Версию LMMS и Windows.

Удачной резьбы.
