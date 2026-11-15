"""Bounded CSI filtering reference for the whofi firmware architecture."""


def filter_frame(values, minimum=-32768, maximum=32767):
    cleaned = [value for value in values if minimum <= value <= maximum]
    return cleaned if cleaned else None


def moving_average(history, width):
    if not history or width <= 0:
        return []
    rows = history[-width:]
    size = min(len(row) for row in rows)
    return [sum(row[index] for row in rows) / len(rows) for index in range(size)]
