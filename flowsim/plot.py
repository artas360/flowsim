from matplotlib.pyplot import figure, show


def simple_time_plot(result, data_key):
    data = result.get_snapshots(data_key, result.general_key)
    data.sort(key=lambda x: x[0])

    fig = figure()
    ax = fig.add_subplot(111)
    # fig.delaxes(ax)
    ax.plot([x[0] for x in data],
            [x[1] for x in data],
            marker='o',
            label=str(data_key)+'=f(time)')
    ax.set_xlabel('time')
    ax.set_ylabel(data_key)
    show()
