# Symulacja zderzeń pp przy √s = 13.6 TeV

Symulacja i analiza rozkładów cząstek w zderzeniach proton–proton przy energii środka masy √s = 13.6 TeV. Pythia 8 generuje zdarzenia minimum bias (`SoftQCD:all`). Program w C++17 i ROOT 6 wybiera cząstki stanu końcowego, liczy krotność oraz rozkłady pT, η i φ i zapisuje histogramy do `analysis.root`. Makro `draw_plots.C` eksportuje wykresy do PNG i PDF.

## Wymagania

- GNU Make
- `g++` i standard C++17
- Pythia 8.318 
- ROOT 6 

`Makefile` wybiera Pythię przez zmienną `PYTHIA8` lub `pythia8-config`
z `PATH`, a ROOT przez `root-config` z `PATH`.

## Struktura projektu

| Ścieżka | Przeznaczenie |
| --- | --- |
| `Makefile` | Sprawdza zależności, buduje `generate` i `analyze` oraz udostępnia cel `pipeline` |
| `generate.cc` | Generator Pythia. Zapisuje drzewo zdarzeń do `event.root` |
| `analyze.cc` | Czyta `event.root`, wypełnia histogramy i zapisuje `analysis.root` |
| `draw_plots.C` | Czyta `analysis.root` i zapisuje wykresy w `plots/` |
| `event.root` | Wynik generatora: drzewo `events` (`pp_collisions_data`) |
| `analysis.root` | Wynik analizy: 32 histogramy `TH1D` |
| `plots/` | Wykresy PNG i PDF |
| `.vscode/c_cpp_properties.json` | Ścieżki nagłówków Pythia i ROOT oraz standard C++17 |
| `proton-proton_prezentacja_rozkładów.pdf` | Prezentacja rozkładów |

Po kompilacji w katalogu projektu powstają pliki wykonywalne `generate` i `analyze`.

## Proces uruchomienia

Sklonuj repozytorium i przejdź do jego katalogu:

```bash
git clone https://github.com/Kacpers89233/proton-proton_simulation.git
cd proton-proton_simulation
```

Skonfiguruj ROOT i wskaż katalog instalacji Pythii:

```bash
source /ścieżka/do/root/bin/thisroot.sh
export PYTHIA8=/ścieżka/do/pythia8318
```

Sprawdź zależności, skompiluj programy i uruchom cały proces:

```bash
make check
make
make pipeline
```

`make` tworzy programy `generate` i `analyze`. `make pipeline` uruchamia
kolejno generator, analizę i makro tworzące wykresy. `make clean` usuwa
wyłącznie skompilowane programy.

Poszczególne etapy można również uruchomić osobno:

```bash
./generate
./analyze
root -l -b -q draw_plots.C
```

Kolejne etapy tworzą odpowiednio:

- `generate` → `event.root`,
- `analyze` → `analysis.root`,
- `draw_plots.C` → `plots/<nazwa_histogramu>.png` i `plots/<nazwa_histogramu>.pdf`.

Programy należy uruchamiać z katalogu głównego, ponieważ korzystają ze ścieżek względnych.

## Konfiguracja edytora

Plik `.vscode/c_cpp_properties.json` korzysta ze zmiennych `PYTHIA8`
i `ROOTSYS`, dzięki czemu nie zawiera ścieżek zależnych od komputera.
Konfiguracja jest opcjonalna i nie wpływa na kompilację przez `make`.

Aby narzędzia edytora korzystały z właściwych nagłówków, skonfiguruj
środowisko przed jego uruchomieniem:

```bash
source /ścieżka/do/root/bin/thisroot.sh
export PYTHIA8=/ścieżka/do/pythia8318
```

Następnie uruchom edytor z tego samego terminala. Zmienne zostaną użyte
do odnalezienia nagłówków Pythii i ROOT.

## Uwagi

Ziarno generatora jest stałe: `Random:setSeed = on` i `Random:seed = 100`.

Pythia zwraca kąt azymutalny w przedziale (−π, π]. Przed zapisem do drzewa wartość ujemna jest zamieniana na dodatnią (`std::fabs`). Tak przekształcone φ trafia do histogramu `hPhi_*` o zakresie 0–π i 64 binach.
