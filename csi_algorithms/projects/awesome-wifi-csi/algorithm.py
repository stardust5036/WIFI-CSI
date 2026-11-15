"""Index helper; Awesome-WiFi-CSI contains references, not one algorithm."""


def compare_results(results):
    return {"count": len(results), "algorithms": sorted(set(results))}
