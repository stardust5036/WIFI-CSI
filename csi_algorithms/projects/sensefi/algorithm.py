"""Dataset/model runner boundary for SenseFi, not an embedded classifier."""


def make_window(features, size):
    return [features[index:index + size]
            for index in range(0, len(features) - size + 1)]


def predict_with_model(model, window):
    """Adapter used by a locally trained SenseFi model."""
    return model(window)
