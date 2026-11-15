"""AMAR output contract: unordered activity set and occupancy count."""


def activity_set(probabilities, labels, threshold=0.5):
    active = {label for label, probability in zip(labels, probabilities)
              if probability >= threshold}
    return {"activities": active, "occupancy_count": len(active)}
