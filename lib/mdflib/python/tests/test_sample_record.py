import pymdf
import logging
import time


def test_sample_record():
    logging.info("TESTING pymdf::SampleRecord")

    sample = pymdf.SampleRecord()
    assert sample is not None

    orig_time = time.time_ns()
    sample.timestamp = orig_time
    assert sample.timestamp == orig_time

    orig_record_id = 12
    sample.record_id = orig_record_id
    assert sample.record_id == orig_record_id

    orig_list = [1, 2, 3, 255]
    sample.data = orig_list
    assert sample.data == orig_list

    orig_data = [1, 2, 3, 4]
    sample.vlsd = True
    sample.vlsd_data = orig_data
    assert sample.vlsd
    assert sample.vlsd_data == orig_data

