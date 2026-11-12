import random
from collections.abc import Callable
from dataclasses import dataclass
from statistics import fmean, quantiles
from time import perf_counter
from typing import Generic, TypeVar

T = TypeVar("T")


@dataclass(frozen=True)
class Benchmark(Generic[T]):
    results: tuple[T, ...]
    runtimes: tuple[float, ...]
    mean: float
    confidence_interval: tuple[float, float]


def benchmark(
    operation: Callable[[], T],
    runs: int,
    duration: Callable[[T], float] | None = None,
) -> Benchmark[T]:
    if runs < 2:
        raise ValueError("Benchmarking requires at least two runs")

    results = []
    runtimes = []
    for _ in range(runs):
        start = perf_counter()
        result = operation()
        elapsed = perf_counter() - start
        results.append(result)
        runtimes.append(elapsed if duration is None else duration(result))

    generator = random.Random(0)
    means = [fmean(generator.choices(runtimes, k=runs)) for _ in range(2000)]
    limits = quantiles(means, n=40, method="inclusive")
    return Benchmark(
        tuple(results),
        tuple(runtimes),
        fmean(runtimes),
        (limits[0], limits[-1]),
    )
