"""CSI-Agent safety boundary: adapt only when evidence passes a no-harm check."""


def choose_action(class_errors, candidate_actions, max_error_increase=0.0):
    selected = {}
    for label, before in class_errors.items():
        options = candidate_actions.get(label, ())
        safe = [item for item in options
                if item["error"] - before <= max_error_increase]
        if safe:
            selected[label] = min(safe, key=lambda item: item["error"])
    return selected
