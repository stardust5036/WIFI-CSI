"""Evaluation guard from the data-leakage paper: split by subject/session."""


def grouped_split(records, group_key="subject", test_groups=()):
    test_groups = set(test_groups)
    train = [record for record in records if record[group_key] not in test_groups]
    test = [record for record in records if record[group_key] in test_groups]
    return train, test
