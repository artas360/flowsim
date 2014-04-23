from matplotlib.pyplot import figure


def simple_time_plot(result, data_key, time_key="elapsed_time"):
    snapshots = result.get_snapshots()
    data = []
    for x in snapshots.itervalues():
        try:
            data.append((x["general"][time_key], x["general"][data_key]))
        except KeyError:
            continue
    data.sort(key=lambda x: x[0])

    fig = figure()
    ax = fig.add_subplot(111)
    # fig.delaxes(ax)
    ax.plot([x[0] for x in data],
            [x[1] for x in data],
            marker='o',
            label=str(data_key)+'=f('+str(time_key)+')')
    ax.set_xlabel(time_key)
    ax.set_ylabel(data_key)
